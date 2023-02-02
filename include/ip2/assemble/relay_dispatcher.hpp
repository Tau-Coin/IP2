/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_RELAY_DISPATCHER_HPP
#define IP2_ASSEMBLE_RELAY_DISPATCHER_HPP

#include "ip2/assemble/getter.hpp"
#include "ip2/assemble/relayer.hpp"
#include "ip2/assemble/assemble_logger.hpp"

#include "ip2/transport/relay_listener.hpp"

#include <ip2/kademlia/item.hpp>
#include <ip2/kademlia/types.hpp>

#include <ip2/config.hpp>

using namespace ip2::transport;

namespace ip2 {
namespace assemble {

struct TORRENT_EXTRA_EXPORT relay_dispatcher final : transport::relay_listener
{
public:

	relay_dispatcher(getter& g, relayer& r, assemble_logger& logger);

	void on_dht_relay(dht::public_key const& from, entry const& payload) override;

private:

	assemble_logger& m_logger;

	getter& m_getter;
	relayer& m_relayer;
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_RELAY_DISPATCHER_HPP
