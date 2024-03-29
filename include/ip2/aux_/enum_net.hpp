/*

Copyright (c) 2007-2008, 2010, 2014-2020, Arvid Norberg
Copyright (c) 2016-2018, 2020-2021, Alden Torres
Copyright (c) 2017, Steven Siloti
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_ENUM_NET_HPP_INCLUDED
#define TORRENT_ENUM_NET_HPP_INCLUDED

#include "ip2/config.hpp"

#include "ip2/aux_/disable_warnings_push.hpp"

#if TORRENT_USE_IFCONF || TORRENT_USE_NETLINK || TORRENT_USE_SYSCTL
#include <sys/socket.h> // for SO_BINDTODEVICE
#endif

#include <optional>

#include "ip2/aux_/disable_warnings_pop.hpp"

#include "ip2/io_context.hpp"
#include "ip2/address.hpp"
#include "ip2/error_code.hpp"
#include "ip2/socket.hpp"
#include "ip2/aux_/bind_to_device.hpp"
#include "ip2/span.hpp"
#include "ip2/flags.hpp"

#include <vector>

namespace ip2::aux {

	// internal
using interface_flags = flags::bitfield_flag<std::uint32_t, struct interface_flags_tag>;

namespace if_flags {

	// internal
	constexpr interface_flags up = 0_bit;
	constexpr interface_flags broadcast = 1_bit;
	constexpr interface_flags loopback = 2_bit;
	constexpr interface_flags pointopoint = 3_bit;
	constexpr interface_flags running = 4_bit;
	constexpr interface_flags noarp = 5_bit;
	constexpr interface_flags promisc = 6_bit;
	constexpr interface_flags allmulti = 7_bit;
	constexpr interface_flags master = 8_bit;
	constexpr interface_flags slave = 9_bit;
	constexpr interface_flags multicast = 10_bit;
	constexpr interface_flags dynamic = 11_bit;
	constexpr interface_flags lower_up = 12_bit;
	constexpr interface_flags dormant = 13_bit;
}

// internal
enum class if_state : std::uint8_t {

	up,
	dormant,
	lowerlayerdown,
	down,
	notpresent,
	testing,
	unknown
};

// internal
	struct ip_interface
	{
		address interface_address;
		address netmask;
		char name[64]{};
		char friendly_name[128]{};
		char description[128]{};
		// an interface is preferred if its address is
		// not tentative/duplicate/deprecated
		bool preferred = true;

		interface_flags flags = if_flags::up;
		if_state state = if_state::unknown;
		
		//added for stats
		unsigned long int rx_bytes = 0;
		unsigned long int tx_bytes = 0;
		unsigned long int rx_errors = 0;
		unsigned long int tx_errors = 0;
		unsigned long int rx_dropped = 0;
		unsigned long int tx_dropped = 0;
	};

// internal
	struct ip_route
	{
		address destination;
		address netmask;
		address gateway;
		address source_hint;
		char name[64]{};
		int mtu = 0;
	};

	// returns a list of the configured IP interfaces
	// on the machine
	TORRENT_EXTRA_EXPORT std::vector<ip_interface> enum_net_interfaces(io_context& ios
		, error_code& ec);

	TORRENT_EXTRA_EXPORT std::vector<ip_route> enum_routes(io_context& ios
		, error_code& ec);

	// returns AF_INET or AF_INET6, depending on the address' family
	TORRENT_EXTRA_EXPORT int family(address const& a);

	// return (a1 & mask) == (a2 & mask)
	TORRENT_EXTRA_EXPORT bool match_addr_mask(address const& a1
		, address const& a2, address const& mask);

	// return a netmask with the specified address family and the specified
	// number of prefix bit set, of the most significant bits in the resulting
	// netmask
	TORRENT_EXTRA_EXPORT address build_netmask(int bits, int family);

	// return the gateway for the given ip_interface, if there is one. Otherwise
	// return nullopt.
	TORRENT_EXTRA_EXPORT std::optional<address> get_gateway(
		ip_interface const& iface, span<ip_route const> routes);

	// returns whether there is a route to the specified device for for any global
	// internet address of the specified address family.
	TORRENT_EXTRA_EXPORT bool has_internet_route(string_view device, int family
		, span<ip_route const> routes);

	// attempt to bind socket to the device with the specified name. For systems
	// that don't support SO_BINDTODEVICE the socket will be bound to one of the
	// IP addresses of the specified device. In this case it is necessary to
	// verify the local endpoint of the socket once the connection is established.
	// the returned address is the ip the socket was bound to (or address_v4::any()
	// in case SO_BINDTODEVICE succeeded and we don't need to verify it).
	// TODO: 3 use string_view for device_name
	template <class Socket>
	address bind_socket_to_device(io_context& ios, Socket& sock
		, tcp const& protocol
		, char const* device_name, int port, error_code& ec)
	{
		tcp::endpoint bind_ep(address_v4::any(), std::uint16_t(port));

		address ip = make_address(device_name, ec);
		if (!ec)
		{
			// this is to cover the case where "0.0.0.0" is considered any IPv4 or
			// IPv6 address. If we're asking to be bound to an IPv6 address and
			// providing 0.0.0.0 as the device, turn it into "::"
			if (ip == address_v4::any() && protocol == boost::asio::ip::tcp::v6())
				ip = address_v6::any();
			bind_ep.address(ip);
			// it appears to be an IP. Just bind to that address
			sock.bind(bind_ep, ec);
			return bind_ep.address();
		}

		ec.clear();

#if TORRENT_HAS_BINDTODEVICE
		// try to use SO_BINDTODEVICE here, if that exists. If it fails,
		// fall back to the mechanism we have below
		aux::bind_device(sock, device_name, ec);
		if (ec)
#endif
		{
			ec.clear();
			// TODO: 2 this could be done more efficiently by just looking up
			// the interface with the given name, maybe even with if_nametoindex()
			std::vector<ip_interface> ifs = enum_net_interfaces(ios, ec);
			if (ec) return bind_ep.address();

			bool found = false;

			for (auto const& iface : ifs)
			{
				// we're looking for a specific interface, and its address
				// (which must be of the same family as the address we're
				// connecting to)
				if (std::strcmp(iface.name, device_name) != 0) continue;
				if (iface.interface_address.is_v4() != (protocol == boost::asio::ip::tcp::v4()))
					continue;

				bind_ep.address(iface.interface_address);
				found = true;
				break;
			}

			if (!found)
			{
				ec = error_code(boost::system::errc::no_such_device, generic_category());
				return bind_ep.address();
			}
		}
		sock.bind(bind_ep, ec);
		return bind_ep.address();
	}

	// returns the device name whose local address is ``addr``. If
	// no such device is found, an empty string is returned.
	TORRENT_EXTRA_EXPORT std::string device_for_address(address addr
		, io_context& ios, error_code& ec);

}

#endif
