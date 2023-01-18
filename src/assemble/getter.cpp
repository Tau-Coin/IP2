/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/getter.hpp"

#include "ip2/aux_/session_interface.hpp"

#include "ip2/kademlia/node_id.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include "ip2/hex.hpp" // to_hex
#endif

#include <ip2/time.hpp>
#include <ip2/aux_/time.hpp>
#include <ip2/api/dht_rpc_params.hpp>

using namespace std::placeholders;

namespace ip2 {

namespace assemble {

getter::getter(io_context& ios
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

void getter::update_node_id()
{
	sha256_hash node_id = dht::get_node_id(m_settings);
	std::memcpy(m_self_pubkey.bytes.data(), node_id.data(), dht::public_key::len);
}

api::error_code getter::get_blob(dht::public_key const& sender
	, aux::uri blob_uri, dht::timestamp ts)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_sender[65];
	char hex_uri[41];
	aux::to_hex(sender.bytes, hex_sender);
	aux::to_hex(blob_uri.bytes, hex_uri);
#endif

	// check network, if dht live nodes is 0, return error.
	if (m_session.dht_nodes() == 0)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR
			, "drop get req:%s/%s, error: dht nodes 0", hex_sender, hex_uri);
#endif

		return api::DHT_LIVE_NODES_ZERO;
	}

	// check transport queue cache size.
	// if transport queue doesn't have enough queue, return error.
	if (!m_session.transporter()->has_enough_buffer(1))
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR
			, "drop get req:%s/%s, error: buffer is full", hex_sender, hex_uri);
#endif

		return api::TRANSPORT_BUFFER_FULL;
	}

	std::shared_ptr<get_context> ctx = std::make_shared<get_context>(
		m_logger, sender, blob_uri, ts);
	api::dht_rpc_params config = get_rpc_parmas(api::GET);
	std::string salt(blob_uri.bytes.data(), 20);
	sha1_hash seg_hash(blob_uri.bytes.data());

	api::error_code result = m_session.transporter()->get(sender
		, salt, ts.value
		, std::bind(&getter::get_callback, self(), _1, _2, ctx, seg_hash)
		, config.invoke_branch, config.invoke_window, config.invoke_limit);

	if (result == api::NO_ERROR)
	{
		ctx->start_getting_seg(seg_hash);
		m_running_tasks.insert(ctx);
	}
    else
    {
        ctx->set_error(result);
        ctx->done();
    }

	return result;
}

void getter::on_incoming_relay_request(dht::public_key const& sender
	, aux::uri blob_uri, dht::timestamp ts)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_sender[65];
	char hex_uri[41];
	aux::to_hex(sender.bytes, hex_sender);
	aux::to_hex(blob_uri.bytes, hex_uri);
#endif

#ifndef TORRENT_DISABLE_LOGGING
	m_logger.log(aux::LOG_INFO
		, "incoming relay uri: sender: %s, uri:%s"
		, hex_sender, hex_uri);
#endif

	//TODO: post "incoming_relay_data_uri_alert"
}

void getter::get_callback(dht::item const& it, bool auth
	, std::shared_ptr<get_context> ctx, sha1_hash seg_hash)
{
	if (!auth) return;

#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(seg_hash, hex_hash);
#endif

	ctx->on_arrived(seg_hash);

	if (ctx->is_root_index(seg_hash))
	{
		// this item is root index protocol
		api::error_code err = ctx->on_root_index_got(it);
		if (err != api::NO_ERROR)
		{
			if (ctx->is_getting_allowed(seg_hash))
			{
#ifndef TORRENT_DISABLE_LOGGING
				m_logger.log(aux::LOG_WARNING, "[%u] re-get index again: %s"
					, ctx->id(), hex_hash);
#endif

				std::string salt(seg_hash.data(), 20);
				api::dht_rpc_params config = get_rpc_parmas(api::GET);

				api::error_code ok = m_session.transporter()->get(ctx->get_sender()
					, salt, ctx->get_timestamp()
					, std::bind(&getter::get_callback, self(), _1, _2, ctx, seg_hash)
					, config.invoke_branch, config.invoke_window, config.invoke_limit);

				if (ok == api::NO_ERROR)
				{
					ctx->start_getting_seg(seg_hash);
				}
				else
				{
					ctx->set_error(ok);
					ctx->done();
					// TODO: post get fail alert
					m_running_tasks.erase(ctx);

					return;
				}
			}
			else
			{
#ifndef TORRENT_DISABLE_LOGGING
				m_logger.log(aux::LOG_ERR, "[%u] getting index failed too many times:%s"
					, ctx->id(), hex_hash);
#endif
				ctx->set_error(err);
				ctx->done();
				// TODO: post get alert
				m_running_tasks.erase(ctx);
				return;
			}
		}
		else
		{
			// get all blob segments
			std::vector<sha1_hash> seg_hashes;
			ctx->get_root_index(seg_hashes);

			if (seg_hashes.size() == 0)
			{
#ifndef TORRENT_DISABLE_LOGGING
				m_logger.log(aux::LOG_ERR, "[%u] empty segment index:%s"
					, ctx->id(), hex_hash);
#endif
                ctx->set_error(api::EMPTY_BLOB_INDEX);
                ctx->done();
                // TODO: post get alert
                m_running_tasks.erase(ctx);
                return;
			}
			else
			{
				// get all blob segments
				// check network, if dht live nodes is 0, return error.
				if (m_session.dht_nodes() == 0)
				{
#ifndef TORRENT_DISABLE_LOGGING
					m_logger.log(aux::LOG_ERR
						, "[%u] drop get request:%s, dht live nodes is 0"
						, ctx->id(), hex_hash);
#endif
					ctx->set_error(api::DHT_LIVE_NODES_ZERO);
					ctx->done();
					// TODO: post get alert
					m_running_tasks.erase(ctx);
					return;
				}

				// check transport queue cache size.
				// if transport queue doesn't have enough queue, return error.
				if (!m_session.transporter()->has_enough_buffer(seg_hashes.size()))
				{
#ifndef TORRENT_DISABLE_LOGGING
					m_logger.log(aux::LOG_ERR
						, "[%u] drop get request:%s, buffer is full:%d"
						, ctx->id(), hex_hash, seg_hashes.size());
#endif
					ctx->set_error(api::TRANSPORT_BUFFER_FULL);
					ctx->done();
					// TODO: post get alert
					m_running_tasks.erase(ctx);
					return;
				}

				api::dht_rpc_params config = get_rpc_parmas(api::GET);

				for (auto& s : seg_hashes)
				{
					std::string seg_salt(s.data(), 20);

					api::error_code ok = m_session.transporter()->get(ctx->get_sender()
						, seg_salt, ctx->get_timestamp()
						, std::bind(&getter::get_callback, self(), _1, _2, ctx, s)
						, config.invoke_branch, config.invoke_window
						, config.invoke_limit);

					if (ok == api::NO_ERROR)
					{
						ctx->start_getting_seg(s);
					}
					else
					{
						ctx->set_error(ok);
						// how to handle this error?
						// if no flying request, done this get task.
						// else wait for reponse.
						if (ctx->is_done())
						{
							ctx->done();
							// TODO: post get failed alert
							m_running_tasks.erase(ctx);
							return;
						}

						break;
					}
				}
			}
		}
	}
	else
	{
		// this item is blob segment
		api::error_code err = ctx->on_segment_got(it, seg_hash);

		if (err != api::NO_ERROR)
		{
			if (ctx->is_getting_allowed(seg_hash))
			{
#ifndef TORRENT_DISABLE_LOGGING
				m_logger.log(aux::LOG_WARNING, "[%u] re-get segment again:%s"
					, ctx->id(), hex_hash);
#endif

				std::string salt(seg_hash.data(), 20);
				api::dht_rpc_params config = get_rpc_parmas(api::GET);

				api::error_code ok = m_session.transporter()->get(ctx->get_sender()
					, salt, ctx->get_timestamp()
					, std::bind(&getter::get_callback, self(), _1, _2, ctx, seg_hash)
					, config.invoke_branch, config.invoke_window, config.invoke_limit);

				if (ok == api::NO_ERROR)
				{
					ctx->start_getting_seg(seg_hash);
				}
				else
				{
					ctx->set_error(ok);
				}
			}
			else
			{
#ifndef TORRENT_DISABLE_LOGGING
				m_logger.log(aux::LOG_ERR, "[%u] getting segment failed too many times:%s"
					, ctx->id(), hex_hash);
#endif
				ctx->set_error(err);
			}
		}

		if (ctx->is_done())
		{
			if (ctx->get_error() != api::NO_ERROR)
			{
				// TODO: post get failed alert
			}
			else
			{
				// TODO: post get successfully alert
				std::string blob;
				bool result = ctx->get_segments_blob(blob);
				if (result)
				{
					// TODO: post get blob alert
				}
				else
				{
#ifndef TORRENT_DISABLE_LOGGING
					m_logger.log(aux::LOG_ERR, "[%u] get broken blob:%s"
						, ctx->id(), hex_hash);
#endif
				}
			}

			ctx->done();
			m_running_tasks.erase(ctx);
		}
	}
}

} // namespace assemble
} // namespace ip2
