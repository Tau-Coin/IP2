/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_RELAYER_HPP
#define IP2_ASSEMBLE_RELAYER_HPP

#include "ip2/assemble/assemble_logger.hpp"
#include "ip2/assemble/relay_context.hpp"
#include "ip2/assemble/rpc_params_config.hpp"

#include <ip2/entry.hpp>
#include <ip2/io_context.hpp>
#include "ip2/api/error_code.hpp"
#include "ip2/aux_/session_interface.hpp"
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
#include <vector>

using namespace ip2::api;

namespace ip2 {

namespace aux {
    struct session_settings;
}

namespace assemble {

class TORRENT_EXTRA_EXPORT relayer final
	: std::enable_shared_from_this<relayer>
{

public:

	relayer(io_context& ios
		, aux::session_interface& session
		, aux::session_settings const& settings
		, counters& cnt
		, assemble_logger& logger);

	relayer(relayer const&) = delete;
	relayer& operator=(relayer const&) = delete;
	relayer(relayer&&) = delete;
	relayer& operator=(relayer&&) = delete;

	std::shared_ptr<relayer> self() { return shared_from_this(); }

	std::tuple<sha256_hash, api::error_code> relay_message(
		dht::public_key const& receiver, span<char const> message);

	void on_incoming_relay_message(dht::public_key const& pk, std::string const& msg);

	void update_node_id();

private:

	void send_callback(entry const& payload
		, std::vector<std::pair<dht::node_entry, bool>> const& nodes
		, std::shared_ptr<relay_context> ctx);

	io_context& m_ios;
	aux::session_interface& m_session;
	aux::session_settings const& m_settings;
	counters& m_counters;

	assemble_logger& m_logger;

	dht::public_key m_self_pubkey;

	std::set<std::shared_ptr<relay_context> > m_running_tasks;

};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_RELAYER_HPP
