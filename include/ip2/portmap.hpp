/*

Copyright (c) 2017-2020, Arvid Norberg
Copyright (c) 2018, Steven Siloti
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_PORTMAP_HPP_INCLUDED
#define TORRENT_PORTMAP_HPP_INCLUDED

#include "ip2/config.hpp"
#include "ip2/units.hpp"

namespace ip2 {

	enum class portmap_transport : std::uint8_t
	{
		// natpmp can be NAT-PMP or PCP
		natpmp, upnp
	};

	enum class portmap_protocol : std::uint8_t
	{
		none, tcp, udp
	};

	// this type represents an index referring to a port mapping
	using port_mapping_t = aux::strong_typedef<int, struct port_mapping_tag>;

}

#endif  //TORRENT_PORTMAP_HPP_INCLUDED
