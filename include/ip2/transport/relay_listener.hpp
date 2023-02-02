/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_TRANSPORT_RELAY_LISTENER_HPP
#define IP2_TRANSPORT_RELAY_LISTENER_HPP

#include "ip2/entry.hpp"

#include <ip2/kademlia/node_id.hpp>

namespace ip2 {
namespace transport {

struct TORRENT_EXPORT relay_listener
{
	virtual void on_dht_relay(dht::public_key const& from, entry const& payload) = 0;
};

} // namespace transport
} // namespace ip2

#endif // IP2_TRANSPORT_RELAY_LISTENER_HPP
