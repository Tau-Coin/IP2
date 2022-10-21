/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/transport/congestion_controller.hpp"

namespace ip2 {

namespace transport {

congestion_controller::congestion_controller(aux::session_interface& session
	, aux::session_settings const& settings
	, transport_logger& logger)
	: m_session(session)
	, m_settings(settings)
	, m_logger(logger)
{
	m_invoking_interval = m_settings.get_int(settings_pack::transport_invoking_interval);
}

int congestion_controller::get_invoking_interval()
{
	return m_invoking_interval;
}

} // namespace transport
} // namespace ip2
