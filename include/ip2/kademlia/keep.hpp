/*

Copyright (c) 2015, Thomas Yuan
Copyright (c) 2016-2020, Arvid Norberg
Copyright (c) 2016, 2018, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_DHT_KEEP_HPP
#define TORRENT_DHT_KEEP_HPP

#include <ip2/kademlia/traversal_algorithm.hpp>
#include <ip2/kademlia/node_entry.hpp>
#include <ip2/kademlia/node_id.hpp>
#include <ip2/kademlia/observer.hpp>

namespace ip2 {
namespace dht {

struct msg;
class node;

struct keep: traversal_algorithm
{

	keep(node& node, node_id const& to);

	char const* name() const override;
	void start() override;

protected:

	void done() override;
	bool invoke(observer_ptr o) override;

	observer_ptr new_observer(udp::endpoint const& ep
		, node_id const& id) override;

	bool m_done = false;
};

struct keep_observer : traversal_observer
{
	keep_observer(
		std::shared_ptr<traversal_algorithm> algorithm
		, udp::endpoint const& ep, node_id const& id)
		: traversal_observer(std::move(algorithm), ep, id)
	{
	}
};

} // namespace dht
} // namespace ip2

#endif // TORRENT_DHT_KEEP_HPP
