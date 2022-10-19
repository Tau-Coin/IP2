/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_BOOTSTRAP_NODES_STORAGE_HPP
#define IP2_BOOTSTRAP_NODES_STORAGE_HPP

#include <functional>

#include <ip2/kademlia/dht_observer.hpp>
#include <ip2/kademlia/node_id.hpp>
#include <ip2/kademlia/types.hpp>
#include <ip2/kademlia/node_entry.hpp>

#include <ip2/aux_/common.h> // for utcTime()

#include <ip2/socket.hpp>
#include <ip2/address.hpp>
#include <ip2/span.hpp>
#include <ip2/string_view.hpp>

namespace ip2 {
namespace dht {

	// This structure hold the basic info of bootstrap node
	struct TORRENT_EXPORT bs_node_entry
	{
		bs_node_entry(node_id const& nid, udp::endpoint const& ep)
		{
			m_nid = nid;
			m_ep = ep;
			m_ts.value = ip2::aux::utcTime();
		}

		bs_node_entry(node_id const& nid, udp::endpoint const& ep, timestamp const& ts)
		{
			m_nid = nid;
			m_ep = ep;
			m_ts = ts;
		}

		// bootstrap node id
		node_id m_nid;

		// ip + port
		udp::endpoint m_ep;

		// sampling timestamp
		timestamp m_ts;
	};

	struct TORRENT_EXPORT bs_nodes_storage_interface
	{
		// Store bootstrap nodes
		virtual bool put(std::vector<bs_node_entry> const& nodes) = 0;

		// Get bootstrap by timestamp priority.
		virtual bool get(std::vector<bs_node_entry>& nodes
			, int offset, int count) const = 0;

		virtual std::size_t size() = 0;

		// This function is called periodically (non-constant frequency).
		//
		// For implementers:
		// Use this functions for expire bootstrap nodes or any other
		// storage cleanup.
		virtual std::size_t tick() = 0;

		// close storage
		virtual void close() = 0;

		// hidden
		virtual ~bs_nodes_storage_interface() {}
	};

} // namespace dht
} // namespace ip2

#endif //IP2_BOOTSTRAP_NODES_STORAGE_HPP
