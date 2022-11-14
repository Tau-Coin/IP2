/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/putter.hpp"
#include "ip2/assemble/protocol.hpp"

#include "ip2/kademlia/node_id.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include "ip2/hex.hpp" // to_hex
#endif

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

std::tuple<sha256_hash, api::error_code> putter::put_blob(span<char const> blob
	, aux::uri const& blob_uri, std::int8_t invoke_limit)
{
	// check blob size
	if (blob.size() > protocol::blob_mtu)
	{
		return std::make_tuple(sha256_hash{}, api::BLOB_TOO_LARGE);
	}

	// check network, if dht live nodes is 0, return error.
	if (m_session.dht_nodes() == 0)
	{
#ifndef TORRENT_DISABLE_LOGGING
		char hex_uri[41];
		aux::to_hex(blob_uri.bytes, hex_uri);

		m_logger.log(aux::LOG_INFO
			, "drop put req:%s, error: dht nodes 0", hex_uri);
#endif

		return std::make_tuple(sha256_hash{}, api::DHT_LIVE_NODES_ZERO);
	}

	std::uint32_t n = static_cast<std::uint32_t>(blob.size()) / protocol::blob_seg_mtu;
	std::uint32_t l = static_cast<std::uint32_t>(blob.size()) % protocol::blob_seg_mtu;
	std::uint32_t seg_count = (l == 0 ? n : n + 1);

	// check transport queue cache size.
	// if transport queue doesn't have enough queue, return error.
	if (!m_session.transporter()->has_enough_buffer(seg_count))
	{
#ifndef TORRENT_DISABLE_LOGGING
		char hex_uri[41];
		aux::to_hex(blob_uri.bytes, hex_uri);

		m_logger.log(aux::LOG_INFO
			, "drop put req:%s, error: buffer is full", hex_uri);
#endif

		return std::make_tuple(sha256_hash{}, api::TRANSPORT_BUFFER_FULL);
	}

	std::shared_ptr<put_context> ctx = std::make_shared<put_context>(m_logger
		, m_self_pubkey, blob_uri, seg_count);

	// start putting the last blob segment
	std::uint32_t begin = (seg_count - 1) * blob_seg_mtu;
	std::uint32_t end = static_cast<std::uint32_t>(blob.size() - 1);

	std::string last_seg;
	std::memcpy(last_seg.data(), blob.data() + begin, end - begin + 1);
	sha1_hash last_seg_hash = hash(last_seg);

	protocol::blob_seg_protocol p(last_seg, sha1_hash{});
	entry pl = p.to_entry();
	api::dht_rpc_params config = get_rpc_parmas(api::PUT);
	config.invoke_limit = invoke_limit;

	api::error_code ok = m_session.transporter()->put(pl
		, std::string(last_seg_hash.data(), 20)
		, std::bind(&putter::put_callback, self(), _1, _2, ctx, last_seg_hash)
		, config.invoke_branch, config.invoke_window, config.invoke_limit);

	if (ok == api::NO_ERROR)
	{
		ctx->add_invoked_segment_hash(last_seg_hash);
		m_running_tasks.insert(ctx);
	}
	else
	{
		ctx->set_error(ok);
		ctx->done();

		return std::make_tuple(sha256_hash{}, ok);
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

		protocol::blob_seg_protocol proto(last_seg, sha1_hash{});
		entry e = proto.to_entry();

		api::error_code err = m_session.transporter()->put(e
			, std::string(seg_hash.data(), 20) 
			, std::bind(&putter::put_callback, self(), _1, _2, ctx, seg_hash)
			, config.invoke_branch, config.invoke_window, config.invoke_limit);

		if (err == api::NO_ERROR)
		{
			ctx->add_invoked_segment_hash(seg_hash);
		}
		else
		{
			ctx->set_error(err);
			break;
		}   

		seg_count -= 1;
	}

	sha256_hash root;
	std::memcpy(root.data(), m_self_pubkey.bytes.data(), 12);
	std::memcpy(&root[12], blob_uri.bytes.data(), 20);

	return std::make_tuple(root, api::NO_ERROR);
}

void putter::update_node_id()
{
	sha256_hash node_id = dht::get_node_id(m_settings);
	std::memcpy(m_self_pubkey.bytes.data(), node_id.data(), dht::public_key::len);
}

void putter::put_callback(dht::item const& it, int responses
	, std::shared_ptr<put_context> ctx, sha1_hash seg_hash)
{
	ctx->add_callbacked_segment_hash(seg_hash, responses);
	if (responses == 0)
	{
		ctx->set_error(api::PUT_RESPONSE_ZERO);
	}

	if (ctx->is_done())
	{
		ctx->done();
		// TODO: post alert with error code
		m_running_tasks.erase(ctx);
	}
}

sha1_hash putter::hash(std::string const& value)
{
	hasher h(value.data(), value.size());
	return h.final();
}

} // namespace assemble
} // namespace ip2
