/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/putter.hpp"
#include "ip2/assemble/protocol.hpp"

#include "ip2/aux_/session_interface.hpp"
#include "ip2/aux_/alert_manager.hpp" // for alert_manager

#include "ip2/kademlia/node_id.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include "ip2/hex.hpp" // to_hex
#endif

#include <algorithm>

using namespace std::placeholders;
using namespace ip2::assemble::protocol;

namespace ip2 {

namespace assemble {

putter::putter(io_context& ios
	, aux::session_interface& session
	, aux::session_settings const& settings
	, counters& cnt
	, assemble_logger& logger)
	: m_ios(ios)
	, m_session(session)
	, m_settings(settings)
	, m_counters(cnt)
	, m_logger(logger)
{
	update_node_id();
}

api::error_code putter::put_blob(span<char const> blob, aux::uri const& blob_uri)
{
	// check blob size
	if (blob.size() > protocol::blob_mtu)
	{
		return api::BLOB_TOO_LARGE;
	}

#ifndef TORRENT_DISABLE_LOGGING
	char hex_uri[41];
	aux::to_hex(blob_uri.bytes, hex_uri);
#endif

	// check network, if dht live nodes is 0, return error.
	if (m_session.dht_nodes() == 0)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_INFO
			, "drop put req:%s, error: dht nodes 0", hex_uri);
#endif

		return api::DHT_LIVE_NODES_ZERO;
	}

	std::uint32_t n = static_cast<std::uint32_t>(blob.size()) / protocol::blob_seg_mtu;
	std::uint32_t l = static_cast<std::uint32_t>(blob.size()) % protocol::blob_seg_mtu;
	std::uint32_t seg_count = (l == 0 ? n : n + 1);

	std::uint32_t buffer_slot = seg_count + 1;

	// check transport queue cache size.
	// if transport queue doesn't have enough queue, return error.
	if (!m_session.transporter()->has_enough_buffer(buffer_slot))
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_INFO
			, "drop put req:%s, error: buffer is full", hex_uri);
#endif

		return api::TRANSPORT_BUFFER_FULL;
	}

	std::shared_ptr<put_context> ctx = std::make_shared<put_context>(m_logger
		, m_self_pubkey, blob_uri, seg_count);
	std::vector<sha1_hash> blob_seg_hashes;

	// start putting the last blob segment
	std::uint32_t begin = (seg_count - 1) * blob_seg_mtu;
	std::uint32_t end = static_cast<std::uint32_t>(blob.size() - 1);

	std::string last_seg;
	std::memcpy(last_seg.data(), blob.data() + begin, end - begin + 1);
	sha1_hash last_seg_hash = hash(last_seg);

	protocol::blob_seg_protocol p(last_seg);
	entry pl = p.to_entry();
	api::dht_rpc_params config = get_rpc_parmas(api::PUT);

	api::error_code ok = m_session.transporter()->put(pl
		, std::string(last_seg_hash.data(), 20)
		, std::bind(&putter::put_callback, self(), _1, _2, ctx, last_seg_hash, true)
		, config.invoke_branch, config.invoke_window, config.invoke_limit);

	if (ok == api::NO_ERROR)
	{
		blob_seg_hashes.push_back(last_seg_hash);
		ctx->add_invoked_hash(last_seg_hash, true);
		m_running_tasks.insert(ctx);
	}
	else
	{
		ctx->set_error(ok);
		ctx->done();

		return ok;
	}

	seg_count -= 1;

	// put left segments
	while (seg_count > 0)
	{
		begin = (seg_count - 1) * blob_seg_mtu;
		end = seg_count * blob_seg_mtu;

		std::string seg;
		std::memcpy(seg.data(), blob.data() + begin, end - begin); 
		sha1_hash seg_hash = hash(seg);

		protocol::blob_seg_protocol proto(seg);
		entry e = proto.to_entry();

		api::error_code err = m_session.transporter()->put(e
			, std::string(seg_hash.data(), 20) 
			, std::bind(&putter::put_callback, self(), _1, _2, ctx, seg_hash, true)
			, config.invoke_branch, config.invoke_window, config.invoke_limit);

		if (err == api::NO_ERROR)
		{
			blob_seg_hashes.push_back(seg_hash);
			ctx->add_invoked_hash(seg_hash, true);
		}
		else
		{
			ctx->set_error(err);
			break;
		}

		seg_count -= 1;
	}

	// if no error, put root index
	if (seg_count == 0 && ctx->get_error() == api::NO_ERROR)
	{
		std::reverse(std::begin(blob_seg_hashes), std::end(blob_seg_hashes));
		for (auto& h : blob_seg_hashes)
		{
			ctx->add_root_index(h);
		}

		protocol::blob_index_protocol rip(blob_seg_hashes);
		entry ripe = rip.to_entry();
		sha1_hash uri_hash(blob_uri.bytes.data());

		api::error_code err = m_session.transporter()->put(ripe
			, std::string(uri_hash.data(), 20)
			, std::bind(&putter::put_callback, self(), _1, _2, ctx, uri_hash, false)
			, config.invoke_branch, config.invoke_window, config.invoke_limit);

		if (err == api::NO_ERROR)
		{
			ctx->add_invoked_hash(uri_hash, false);
        }
		else
		{
			ctx->set_error(err);
		}
	}

	// if the first segment failed, directly return error
	if (ctx->is_done())
	{
		api::error_code ret_error = ctx->get_error();
		ctx->done();
		m_running_tasks.erase(ctx);

		return ret_error;
	}

	return api::NO_ERROR;
}

void putter::update_node_id()
{
	sha256_hash node_id = dht::get_node_id(m_settings);
	std::memcpy(m_self_pubkey.bytes.data(), node_id.data(), dht::public_key::len);
}

void putter::put_callback(dht::item const& it, int responses
	, std::shared_ptr<put_context> ctx, sha1_hash h, bool is_seg)
{
	ctx->add_callbacked_hash(h, responses, is_seg);
	if (responses == 0)
	{
		if (ctx->is_reput_allowed(h))
		{
			api::dht_rpc_params config = get_rpc_parmas(api::PUT);

			api::error_code err = m_session.transporter()->put(it.value()
				, std::string(h.data(), 20)
				, std::bind(&putter::put_callback, self(), _1, _2, ctx, h, is_seg)
				, config.invoke_branch, config.invoke_window, config.invoke_limit);

			if (err == api::NO_ERROR)
			{
				ctx->add_invoked_hash(h, is_seg);
			}
			else
			{
				ctx->set_error(err);
			}
		}
		else
		{
			ctx->set_error(api::PUT_RESPONSE_ZERO);
		}
	}

	if (ctx->is_done())
	{
		ctx->done();
		// post alert with error code
		aux::uri data_uri = ctx->get_uri();
		m_session.alerts().emplace_alert<put_data_alert>(data_uri.bytes.data()
			, ctx->get_error());
		m_running_tasks.erase(ctx);
	}
}

sha1_hash putter::hash(std::string const& value)
{
	hasher h(value.data(), value.size());
	return h.final();
}

sha1_hash putter::hash(std::vector<sha1_hash> const& hl)
{
	hasher h;

	for (auto it = hl.begin(); it != hl.end(); it++)
	{
		h.update(it->data(), 20);
	}

	return h.final();
}

sha1_hash putter::hash(span<char const> blob, aux::uri const& blob_uri)
{
	hasher h;

	h.update(blob.data(), blob.size());
	h.update(blob_uri.bytes.data(), 20);

	return h.final();
}

} // namespace assemble
} // namespace ip2
