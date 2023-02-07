/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_PUTTER_HPP
#define IP2_ASSEMBLE_PUTTER_HPP

#include "ip2/assemble/assemble_logger.hpp"
#include "ip2/assemble/put_context.hpp"
#include "ip2/assemble/rpc_params_config.hpp"

#include <ip2/entry.hpp>
#include <ip2/io_context.hpp>
#include "ip2/api/error_code.hpp"
#include "ip2/aux_/common.h"
#include "ip2/aux_/deadline_timer.hpp"
#include "ip2/span.hpp"
#include "ip2/uri.hpp"

#include <ip2/kademlia/types.hpp>
#include <ip2/kademlia/item.hpp>
#include <ip2/kademlia/node_entry.hpp>

#include <functional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace ip2::api;

namespace ip2 {

	struct counters;

namespace aux {
    struct session_settings;
	struct session_interface;
}

namespace assemble {

class TORRENT_EXTRA_EXPORT putter final
	: public std::enable_shared_from_this<putter>
{
public:
	putter(io_context& ios
		, aux::session_interface& session
		, aux::session_settings const& settings
		, counters& cnt
		, assemble_logger& logger);

	putter(putter const&) = delete;
	putter& operator=(putter const&) = delete;
	putter(putter&&) = delete;
	putter& operator=(putter&&) = delete;

	std::shared_ptr<putter> self() { return shared_from_this(); }

	api::error_code put_blob(span<char const> blob, aux::uri const& blob_uri);

	void put_callback(dht::item const& it, int responses
		, std::shared_ptr<put_context> ctx, sha1_hash hash, bool is_seg);

	void update_node_id();

private:

	sha1_hash hash(std::string const& value);

	sha1_hash hash(std::vector<sha1_hash> const& hl);

	sha1_hash hash(span<char const> blob, aux::uri const& blob_uri);

	io_context& m_ios;
	aux::session_interface& m_session;
	aux::session_settings const& m_settings;
	counters& m_counters;

	assemble_logger& m_logger;

	dht::public_key m_self_pubkey;

	std::set<std::shared_ptr<put_context> > m_running_tasks;
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_PUTTER_HPP
