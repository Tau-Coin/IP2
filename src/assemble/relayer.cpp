/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/relayer.hpp"
#include "ip2/assemble/protocol.hpp"

#include "ip2/aux_/session_interface.hpp"

#include "ip2/kademlia/node_id.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include "ip2/hex.hpp" // to_hex
#endif

#include "ip2/hasher.hpp"

using namespace std::placeholders;
using namespace ip2::assemble::protocol;

namespace ip2 {

namespace assemble {

relayer::relayer(io_context& ios
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

void relayer::update_node_id()
{
	sha256_hash node_id = dht::get_node_id(m_settings);
	std::memcpy(m_self_pubkey.bytes.data(), node_id.data(), dht::public_key::len);
}

api::error_code relayer::relay_message(dht::public_key const& receiver
	, span<char const> message)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_key[65];
	aux::to_hex(receiver.bytes, hex_key);
#endif

	// check network, if dht live nodes is 0, return error.
	if (m_session.dht_nodes() == 0)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR
			, "drop relay message:%s, error: dht nodes 0", hex_key);
#endif

		return api::DHT_LIVE_NODES_ZERO;
	}

	// check transport queue cache size.
	// if transport queue doesn't have enough queue, return error.
	if (!m_session.transporter()->has_enough_buffer(1))
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR
			, "drop relay message:%s, error: buffer is full", hex_key);
#endif

		return api::TRANSPORT_BUFFER_FULL;
	}

	// check message size
	if ((int)message.size() > protocol::relay_msg_mtu)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR
			, "drop relay message:%s, too large:%d", hex_key, (int)message.size());
#endif

		return api::BLOB_TOO_LARGE;
	}

	hasher h(message);
	h.update(receiver.bytes.data(), 32);
	sha1_hash msg_id = h.final();

	std::shared_ptr<relay_context> ctx = std::make_shared<relay_context>(m_logger
		, receiver, msg_id, MESSAGE);

	protocol::relay_msg_protocol p(std::string(message.data(), message.size()));
	entry pl = p.to_entry();
	api::dht_rpc_params config = get_rpc_parmas(api::RELAY);

	api::error_code ok = m_session.transporter()->send(receiver, pl
		, std::bind(&relayer::send_message_callback, self(), _1, _2, ctx)
		, config.invoke_branch, config.invoke_window
		, config.invoke_limit, config.hit_limit);

	if (ok == api::NO_ERROR)
	{
		ctx->start_relay();
		m_running_tasks.insert(ctx);

		return api::NO_ERROR;
	}
	else
	{
		ctx->set_error(ok);
		ctx->done();

		return ok;
	}
}

api::error_code relayer::relay_uri(dht::public_key const& receiver
	, aux::uri const& data_uri, dht::timestamp ts)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_uri[41];
    char hex_key[65];
	aux::to_hex(data_uri.bytes, hex_uri);
    aux::to_hex(receiver.bytes, hex_key);
#endif

	// check network, if dht live nodes is 0, return error.
	if (m_session.dht_nodes() == 0)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR
		, "drop relay uri:%s/%s, error: dht nodes 0", hex_key, hex_uri);
#endif

		return api::DHT_LIVE_NODES_ZERO;
	}

	// check transport queue cache size.
	// if transport queue doesn't have enough queue, return error.
	if (!m_session.transporter()->has_enough_buffer(1))
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR
			, "drop relay uri:%s/%s, error: buffer is full", hex_key, hex_uri);
#endif

		return api::TRANSPORT_BUFFER_FULL;
	}

	hasher h;
	h.update(receiver.bytes.data(), 32);
	h.update(data_uri.bytes.data(), 20);
	sha1_hash msg_id = h.final();

	std::shared_ptr<relay_context> ctx = std::make_shared<relay_context>(m_logger
		, receiver, msg_id, URI);

	protocol::relay_uri_protocol p(m_self_pubkey, data_uri, ts);
	entry pl = p.to_entry();
	api::dht_rpc_params config = get_rpc_parmas(api::RELAY);

	api::error_code ok = m_session.transporter()->send(receiver, pl
		, std::bind(&relayer::send_uri_callback, self(), _1, _2
			, ctx, receiver, data_uri, ts)
		, config.invoke_branch, config.invoke_window
		, config.invoke_limit, config.hit_limit);

	if (ok == api::NO_ERROR)
	{
		ctx->start_relay();
		m_running_tasks.insert(ctx);

		return api::NO_ERROR;
	}
	else
	{
		ctx->set_error(ok);
		ctx->done();

		return ok;
	}
}

void relayer::on_incoming_relay_message(dht::public_key const& pk, std::string const& msg)
{
	// TODO: post 'incoming_relay_alert'
}

void relayer::send_message_callback(entry const& payload
	, std::vector<std::pair<dht::node_entry, bool>> const& nodes
	, std::shared_ptr<relay_context> ctx)
{
	if (nodes.size() == 0)
	{
		ctx->set_error(api::RELAY_RESPONSE_ZERO);
	}

	ctx->done();

	if (ctx->get_relay_type() == MESSAGE)
	{
		// TODO: post 'relay_message_alert'
	}

	m_running_tasks.erase(ctx);
}

void relayer::send_uri_callback(entry const& payload
	, std::vector<std::pair<dht::node_entry, bool>> const& nodes
	, std::shared_ptr<relay_context> ctx
	, dht::public_key receiver, aux::uri data_uri, dht::timestamp ts)
{
	if (nodes.size() == 0)
	{
		ctx->set_error(api::RELAY_RESPONSE_ZERO);
	}

	ctx->done();

    if (ctx->get_relay_type() == URI)
	{
		// TODO: post 'relay_data_uri_alert'
	}

	m_running_tasks.erase(ctx);
}

} // namespace assemble
} // namespace ip2
