/*

Copyright (c) 2016, 2021, Alden Torres
Copyright (c) 2017, Steven Siloti
Copyright (c) 2017-2020, Arvid Norberg
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/kademlia/dht_state.hpp"

#include <ip2/bdecode.hpp>
#include <ip2/aux_/socket_io.hpp>

namespace ip2::dht {

	node_ids_t extract_node_ids(bdecode_node const& e, string_view key)
	{
		if (e.type() != bdecode_node::dict_t) return node_ids_t();
		node_ids_t ret;
		// first look for an old-style nid
		auto const old_nid = e.dict_find_string_value(key);
		if (old_nid.size() == 32)
		{
			ret.emplace_back(address(), node_id(old_nid));
			return ret;
		}
		auto const nids = e.dict_find_list(key);
		if (!nids) return ret;
		for (int i = 0; i < nids.list_size(); i++)
		{
			bdecode_node nid = nids.list_at(i);
			if (nid.type() != bdecode_node::string_t) continue;
			if (nid.string_length() < 32) continue;
			char const* in = nid.string_ptr();
			node_id id(in);
			in += id.size();
			address addr;
			if (nid.string_length() == 36)
				addr = aux::read_v4_address(in);
			else if (nid.string_length() == 48)
				addr = aux::read_v6_address(in);
			else
				continue;
			ret.emplace_back(addr, id);
		}

		return ret;
	}

namespace {
	entry save_nodes(std::vector<node_entry> const& nodes)
	{
		entry ret(entry::list_t);
		entry::list_type& list = ret.list();
		for (auto const& n : nodes)
		{
			std::string nid;
			std::copy(n.id.begin(), n.id.end(), std::back_inserter(nid));
			aux::write_endpoint(n.ep(), std::back_inserter(nid));
			list.emplace_back(std::move(nid));
		}
		return ret;
	}

	std::vector<node_entry> extract_node_entries(bdecode_node const& e, string_view key)
	{
		std::vector<node_entry> ret;
		if (e.type() != bdecode_node::dict_t) return ret;
		auto const nodes = e.dict_find_list(key);
		if (!nodes) return ret;

		for (int i = 0; i < nodes.list_size(); i++)
		{
			bdecode_node n = nodes.list_at(i);
			if (n.type() != bdecode_node::string_t) continue;
			if (n.string_length() < 32) continue;
			char const* in = n.string_ptr();
			node_id id(in);
			in += id.size();
			udp::endpoint ep;
			if (n.string_length() == 38)
				ep = aux::read_v4_endpoint<udp::endpoint>(in);
			else if (n.string_length() == 50)
				ep = aux::read_v6_endpoint<udp::endpoint>(in);
			else
				continue;

			ret.emplace_back(id, ep);
		}

		return ret;
	}
} // anonymous namespace

	void dht_state::clear()
	{
		nids.clear();
		nids.shrink_to_fit();

		nodes.clear();
		nodes.shrink_to_fit();
		nodes6.clear();
		nodes6.shrink_to_fit();
	}

	dht_state read_dht_state(bdecode_node const& e)
	{
		dht_state ret;

		if (e.type() != bdecode_node::dict_t) return ret;

		ret.nids = extract_node_ids(e, "node-id");

		if (bdecode_node const nodes = e.dict_find_list("nodes"))
			ret.nodes = extract_node_entries(e, "nodes");
		if (bdecode_node const nodes = e.dict_find_list("nodes6"))
			ret.nodes6 = extract_node_entries(e, "nodes6");
		return ret;
	}

	entry save_dht_state(dht_state const& state)
	{
		entry ret(entry::dictionary_t);
		auto& nids = ret["node-id"].list();
		for (auto const& n : state.nids)
		{
			std::string nid;
			std::copy(n.second.begin(), n.second.end(), std::back_inserter(nid));
			aux::write_address(n.first, std::back_inserter(nid));
			nids.emplace_back(std::move(nid));
		}
		entry const nodes = save_nodes(state.nodes);
		if (!nodes.list().empty()) ret["nodes"] = nodes;
		entry const nodes6 = save_nodes(state.nodes6);
		if (!nodes6.list().empty()) ret["nodes6"] = nodes6;
		return ret;
	}
}
