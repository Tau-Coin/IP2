/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_GETTER_HPP
#define IP2_ASSEMBLE_GETTER_HPP

#include "ip2/assemble/assemble_logger.hpp"
#include "ip2/assemble/get_context.hpp"
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
#include <queue>
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

static constexpr int get_tasks_limit = 50;

class TORRENT_EXTRA_EXPORT getter final
	: std::enable_shared_from_this<getter>
{
public:
	getter(io_context& ios
		, aux::session_interface& session
		, aux::session_settings const& settings
		, counters& cnt
		, assemble_logger& logger);

	getter(getter const&) = delete;
	getter& operator=(getter const&) = delete;
	getter(getter&&) = delete;
	getter& operator=(getter&&) = delete;

	std::shared_ptr<getter> self() { return shared_from_this(); }

	api::error_code get_blob(dht::public_key const& sender
		, aux::uri blob_uri, dht::timestamp ts);

	void on_incoming_relay_uri(dht::public_key const& sender
		, aux::uri blob_uri, dht::timestamp ts);

	void update_node_id();

private:

	void get_callback(dht::item const& it, bool auth
		, std::shared_ptr<get_context> ctx, sha1_hash hash, bool is_seg);

	void post_alert(std::shared_ptr<get_context> ctx);

	io_context& m_ios;
	aux::session_interface& m_session;
	aux::session_settings const& m_settings;
	counters& m_counters;

	assemble_logger& m_logger;

	dht::public_key m_self_pubkey;

	std::set<std::shared_ptr<get_context> > m_running_tasks;
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_GETTER_HPP
