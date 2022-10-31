/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/transport/transporter.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include "ip2/hex.hpp" // to_hex
#endif
#include <ip2/time.hpp>
#include "ip2/aux_/alert_manager.hpp" // for alert_manager
#include <ip2/aux_/time.hpp> // for aux::time_now
#include "ip2/kademlia/dht_tracker.hpp"

#include <vector>

using namespace std::placeholders;
using ip2::dht::dht_tracker;

namespace ip2 {

namespace transport {

transporter::transporter(io_context& ios
	, aux::session_interface& session
	, aux::session_settings const& settings
	, counters& cnt)
	: m_ios(ios)
	, m_session(session)
	, m_settings(settings)
	, m_counters(cnt)
	, m_invoking_timer(ios)
	, m_congestion_controller(session, settings, *this)
{
}

bool transporter::should_log(aux::LOG_LEVEL log_level) const
{
	return log_level <= m_session.get_log_level()
		&& m_session.alerts().should_post<transport_log_alert>();
}

TORRENT_FORMAT(3,4)
void transporter::log(aux::LOG_LEVEL log_level, char const* fmt, ...) noexcept try
{
#ifndef TORRENT_DISABLE_LOGGING
	if (!should_log(log_level)) return;

	va_list v;
	va_start(v, fmt);
	m_session.alerts().emplace_alert<transport_log_alert>(fmt, v);
	va_end(v);
#endif
}
catch (std::exception const&)
{}

void transporter::start()
{
	log(aux::LOG_NOTICE, "starting transporter...");

	m_running = true;

	m_invoking_timer.expires_after(
		milliseconds(m_congestion_controller.get_invoking_interval()));
	m_invoking_timer.async_wait(std::bind(&transporter::invoking_timeout, self(), _1));
}

void transporter::stop()
{
	log(aux::LOG_NOTICE, "stopping transporter...");

	m_running = false;
	m_invoking_timer.cancel();
	// clear invoking queue
	std::queue<rpc> empty;
	m_rpc_queue.swap(empty);
}

api::error_code transporter::get(dht::public_key const& key
	, std::string salt
	, std::int64_t timestamp
	, std::function<void(dht::item const&, bool)> cb
	, std::int8_t invoke_branch // alpha
	, std::int8_t invoke_window
	, std::int8_t invoke_limit)
{
	if (!m_running) return api::TRANSPORT_STOPPED;
	if (m_rpc_queue.size() >= (long)m_settings.get_int(
			settings_pack::transport_invoking_interval))
	{
		return api::TRANSPORT_BUFFER_FULL;
	}

#ifndef TORRENT_DISABLE_LOGGING
	char hex_key[65];
	char hex_salt[129]; // 64*2 + 1
	aux::to_hex(key.bytes, hex_key);
	aux::to_hex(salt, hex_salt);
	log(aux::LOG_INFO, "enqueue get req for [k:%s, s:%s, window:%d, limit:%d, qs:%d]"
		, hex_key, hex_salt, invoke_window, invoke_limit
		, (int)m_rpc_queue.size());
#endif

	std::shared_ptr<get_ctx> ctx = std::make_shared<get_ctx>(key, salt, timestamp
		, invoke_branch, invoke_window, invoke_limit);
	std::function<void(dht::item const&, bool)> callback
		= std::bind(&transporter::get_callback, self(), _1, _2, ctx, cb);

	void (dht_tracker::*get)(dht::public_key const& key
		, std::function<void(dht::item const&, bool)> cb
		, std::int8_t alpha, std::int8_t invoke_window, std::int8_t invoke_limit
		, std::string salt, std::int64_t timestamp) = &dht_tracker::get_item;

	rpc_method method = std::bind(get, m_session.dht()->self()
		, ctx->m_pubkey, std::move(callback)
		, invoke_branch, invoke_window, invoke_limit
		, ctx->m_salt, ctx->m_timestamp);
	m_rpc_queue.push(rpc(std::move(method)));

	return api::NO_ERROR;
}

api::error_code transporter::put(entry const& data
	, std::string salt
	, std::function<void(dht::item const&, int)> cb
	, std::int8_t invoke_branch
	, std::int8_t invoke_window
	, std::int8_t invoke_limit)
{
	if (!m_running) return api::TRANSPORT_STOPPED;
	if (m_rpc_queue.size() >= (long)m_settings.get_int(
		settings_pack::transport_invoking_interval))
	{
		return api::TRANSPORT_BUFFER_FULL;
	}

#ifndef TORRENT_DISABLE_LOGGING
	char hex_salt[129]; // 64*2 + 1
	aux::to_hex(salt, hex_salt);
	log(aux::LOG_INFO
		, "enqueue put req [s:%s, window:%d, limit:%d, qs:%d]"
		, hex_salt, invoke_window, invoke_limit, (int)m_rpc_queue.size());
#endif

	std::shared_ptr<put_ctx> ctx = std::make_shared<put_ctx>(data, salt
		, invoke_branch, invoke_window, invoke_limit);
	std::function<void(dht::item const&, int responses)> callback
		= std::bind(&transporter::put_callback, self(), _1, _2, ctx, cb);

	void (dht_tracker::*put)(entry const& data
		, std::function<void(dht::item const&, int)> cb
		, std::int8_t alpha, std::int8_t invoke_window, std::int8_t invoke_limit
		, std::string salt) = &dht_tracker::put_item;

	rpc_method method = std::bind(put, m_session.dht()->self()
		, ctx->m_data, std::move(callback)
		, invoke_branch, invoke_window, invoke_limit, ctx->m_salt);
	m_rpc_queue.push(rpc(std::move(method)));

	return api::NO_ERROR;
}

api::error_code transporter::send(dht::public_key const& to
	, entry const& payload
	, std::function<void(entry const& payload
		, std::vector<std::pair<dht::node_entry, bool>> const& nodes)> cb
	, std::int8_t invoke_branch
	, std::int8_t invoke_window
	, std::int8_t invoke_limit
	, std::int8_t hit_limit)
{
	if (!m_running) return api::TRANSPORT_STOPPED;
	if (m_rpc_queue.size() >= (long)m_settings.get_int(
		settings_pack::transport_invoking_interval))
	{
		return api::TRANSPORT_BUFFER_FULL;
	}

#ifndef TORRENT_DISABLE_LOGGING
	char hex_to[65];
	aux::to_hex(to.bytes, hex_to);
	log(aux::LOG_INFO, "enqueue send req [t:%s, qs:%d]", hex_to
		, (int)m_rpc_queue.size());
#endif

	std::shared_ptr<relay_ctx> ctx = std::make_shared<relay_ctx>(to, payload
		, invoke_branch, invoke_window, invoke_limit);
	std::function<void(entry const&
			, std::vector<std::pair<dht::node_entry, bool>> const& success_nodes)>
		callback = std::bind(&transporter::send_callback, self(), _1, _2, ctx, cb);

	void (dht_tracker::*send)(dht::public_key const& to, entry const& payload
		, std::int8_t alpha, std::int8_t beta , std::int8_t invoke_limit
		, std::int8_t hit_limit
		, std::function<void(entry const& payload
			, std::vector<std::pair<dht::node_entry, bool>> const& nodes)> cb)
		= &dht_tracker::send;

	rpc_method method = std::bind(send, m_session.dht()->self()
		, ctx->m_to, ctx->m_payload
		, invoke_branch, invoke_window, invoke_limit, hit_limit, std::move(callback));
	m_rpc_queue.push(rpc(std::move(method)));

	return api::NO_ERROR;
}

void transporter::get_callback(dht::item const& it, bool authoritative
	, std::shared_ptr<get_ctx> ctx
	, std::function<void(dht::item const&, bool)> f)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_key[65];
	char hex_salt[129]; // 64*2 + 1
	aux::to_hex(ctx->m_pubkey.bytes, hex_key);
	aux::to_hex(ctx->m_salt, hex_salt);
	log(aux::LOG_INFO, "get cb for [ k:%s, s:%s, v:%s]"
		, hex_key, hex_salt, it.value().to_string(true).c_str());
#endif

	f(it, authoritative);
}

void transporter::put_callback(dht::item const& it, int responses
	, std::shared_ptr<put_ctx> ctx
	, std::function<void(dht::item const&, int)> f)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_salt[129]; // 64*2 + 1
	aux::to_hex(ctx->m_salt, hex_salt);
	log(aux::LOG_INFO, "put cb for [s:%s, r:%d]", hex_salt, responses);
#endif

	f(it, responses);
}

void transporter::send_callback(entry const& it
	, std::vector<std::pair<dht::node_entry, bool>> const& success_nodes
	, std::shared_ptr<relay_ctx> ctx
	, std::function<void(entry const&
		, std::vector<std::pair<dht::node_entry, bool>> const&)> f)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_to[65];
	aux::to_hex(ctx->m_to.bytes, hex_to);
	log(aux::LOG_INFO, "send cb for [t:%s, sn:%d]", hex_to, (int)success_nodes.size());
#endif

	f(it, success_nodes);
}

void transporter::invoking_timeout(error_code const& e)
{
	if (e || !m_running) return;

	if (m_rpc_queue.size() > 0 && m_session.dht_nodes() > 0)
	{
		auto const& r = m_rpc_queue.front();
		r.m_method();
		m_rpc_queue.pop();

		m_congestion_controller.tick();
	}

	m_invoking_timer.expires_after(
		milliseconds(m_congestion_controller.get_invoking_interval()));
	m_invoking_timer.async_wait(std::bind(&transporter::invoking_timeout, self(), _1));
}

} // namespace transport
} // namespace ip2
