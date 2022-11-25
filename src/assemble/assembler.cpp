/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/assembler.hpp"

#include "ip2/aux_/alert_manager.hpp" // for alert_manager

namespace ip2 {

namespace assemble {

assembler::assembler(io_context& ios
	, aux::session_interface& session
	, aux::session_settings const& settings
	, counters& cnt)
	: m_ios(ios)
	, m_session(session)
	, m_settings(settings)
	, m_counters(cnt)
	, m_getter(ios, session, settings, cnt, *this)
	, m_putter(ios, session, settings, cnt, *this)
	, m_relayer(ios, session, settings, cnt, *this)
{
	// initialize node id
	sha256_hash node_id = dht::get_node_id(m_settings);
	std::memcpy(m_self_pubkey.bytes.data(), node_id.data(), dht::public_key::len);
}

void assembler::update_node_id()
{
	m_getter.update_node_id();
	m_putter.update_node_id();
	m_relayer.update_node_id();
}

bool assembler::should_log(aux::LOG_LEVEL log_level) const
{
	return log_level <= m_session.get_log_level()
		&& m_session.alerts().should_post<assemble_log_alert>();
}

TORRENT_FORMAT(3,4)
void assembler::log(aux::LOG_LEVEL log_level, char const* fmt, ...) noexcept try 
{
#ifndef TORRENT_DISABLE_LOGGING
	if (!should_log(log_level)) return;

	va_list v;
	va_start(v, fmt);
	m_session.alerts().emplace_alert<assemble_log_alert>(fmt, v); 
	va_end(v);
#endif
}
catch (std::exception const&)
{}

void assembler::start()
{
	if (m_running) return;

	m_running = true;
	m_getter.start();
}

void assembler::stop()
{
	if (!m_running) return;

	m_getter.stop();
	m_running = false;
}

} // namespace assemble
} // namespace ip2
