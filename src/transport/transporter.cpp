/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/transport/transporter.hpp"

#include <ip2/time.hpp>
#include "ip2/aux_/alert_manager.hpp" // for alert_manager
#include <ip2/aux_/time.hpp> // for aux::time_now

using namespace std::placeholders;

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
	m_running = false;
	m_invoking_timer.cancel();
}

rpc_result transporter::get(dht::public_key const& key
	, std::string salt
	, std::int64_t timestamp
	, std::function<void(dht::item const&, bool)> cb
	, std::int8_t invoke_branch // alpha
	, std::int8_t invoke_window
	, std::int8_t invoke_limit)
{
	return ok;
}

rpc_result transporter::put(entry const& data 
	, std::string salt
	, std::function<void(dht::item const&, int)> cb
	, std::int8_t invoke_branch
	, std::int8_t invoke_window
	, std::int8_t invoke_limit)
{
	return ok;
}

rpc_result transporter::send(dht::public_key const& to
	, entry const& payload
	, std::function<void(entry const& payload
		, std::vector<std::pair<dht::node_entry, bool>> const& nodes)> cb
	, std::int8_t invoke_branch
	, std::int8_t invoke_window
	, std::int8_t invoke_limit
	, std::int8_t hit_limit)
{
	return ok;
}

void transporter::get_callback(dht::item const& it, bool authoritative
	, std::shared_ptr<get_ctx> ctx
	, std::function<void(dht::item const&, bool)> f)
{
}

void transporter::put_callback(dht::item const& it, int responses
	, std::shared_ptr<put_ctx> ctx
	, std::function<void(dht::item const&, int)> cb)
{
}

void transporter::send_callback(entry const& it
	, std::vector<std::pair<dht::node_entry, bool>> const& success_nodes
	, std::shared_ptr<relay_ctx> ctx
	, std::function<void(entry const&
		, std::vector<std::pair<dht::node_entry, bool>> const&)> cb)
{
}

void transporter::invoking_timeout(error_code const& e)
{
}

} // namespace transport
} // namespace ip2
