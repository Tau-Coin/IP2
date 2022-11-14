/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_TRANSPORT_CONGESTION_CONTROLLER_HPP
#define IP2_TRANSPORT_CONGESTION_CONTROLLER_HPP

#include "ip2/transport/transport_logger.hpp"

#include "ip2/config.hpp"

namespace ip2 {

namespace aux {
    struct session_settings;
	struct session_interface;
}

namespace transport {

class TORRENT_EXTRA_EXPORT congestion_controller
{
public:
	congestion_controller(aux::session_interface& session
		, aux::session_settings const& settings
		, transport_logger& logger);

	congestion_controller(congestion_controller const&) = delete;
	congestion_controller& operator=(congestion_controller const&) = delete;
	congestion_controller(congestion_controller&&) = delete;
	congestion_controller& operator=(congestion_controller&&) = delete;

	int get_invoking_interval();
	void tick();

private:

	aux::session_interface& m_session;
	aux::session_settings const& m_settings;

	transport_logger& m_logger;

	int m_invoking_interval; // ms unit
};
} // namespace transport
} // namespace ip2

#endif // IP2_TRANSPORT_CONGESTION_CONTROLLER_HPP
