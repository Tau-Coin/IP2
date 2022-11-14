/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_TRANSPORT_TRANSPORTER_HPP
#define IP2_TRANSPORT_TRANSPORTER_HPP

#include "ip2/transport/congestion_controller.hpp"
#include "ip2/transport/dht_rpc.hpp"
#include "ip2/transport/transport_logger.hpp"
#include "ip2/transport/relay_listener.hpp"

#include <ip2/entry.hpp>
#include <ip2/io_context.hpp>
#include "ip2/api/error_code.hpp"
#include "ip2/aux_/common.h"
#include "ip2/aux_/deadline_timer.hpp"

#include <ip2/kademlia/types.hpp>
#include <ip2/kademlia/item.hpp>
#include <ip2/kademlia/node_entry.hpp>

#include <functional>
#include <queue>
#include <set>
#include <string>
#include <vector>

using namespace ip2::api;

namespace ip2 {

	struct counters;

namespace aux {
    struct session_settings;
	struct session_interface;
}

namespace transport {

class TORRENT_EXTRA_EXPORT transporter final
	: std::enable_shared_from_this<transporter>, transport_logger
{
public:
	transporter(io_context& ios
		, aux::session_interface& session
		, aux::session_settings const& settings
		, counters& cnt);

	transporter(transporter const&) = delete;
	transporter& operator=(transporter const&) = delete;
	transporter(transporter&&) = delete;
	transporter& operator=(transporter&&) = delete;

	bool should_log(aux::LOG_LEVEL log_level) const override;
	void log(aux::LOG_LEVEL log_level, char const* fmt, ...)
		noexcept override TORRENT_FORMAT(3,4);

	std::shared_ptr<transporter> self() { return shared_from_this(); }

	void start();
	void stop();

	bool has_enough_buffer(int slots);

	api::error_code get(dht::public_key const& key
		, std::string salt
		, std::int64_t timestamp
		, std::function<void(dht::item const&, bool)> cb
		, std::int8_t invoke_branch
		, std::int8_t invoke_window
		, std::int8_t invoke_limit);

	api::error_code put(entry const& data
		, std::string salt
		, std::function<void(dht::item const&, int)> cb
		, std::int8_t invoke_branch
		, std::int8_t invoke_window
		, std::int8_t invoke_limit);

	api::error_code send(dht::public_key const& to
		, entry const& payload
		, std::function<void(entry const& payload
			, std::vector<std::pair<dht::node_entry, bool>> const& nodes)> cb
		, std::int8_t invoke_branch
		, std::int8_t invoke_window
		, std::int8_t invoke_limit
		, std::int8_t hit_limit);

	// callback

	void get_callback(dht::item const& it, bool authoritative
		, std::shared_ptr<get_ctx> ctx
		, std::function<void(dht::item const&, bool)> f);

	void put_callback(dht::item const& it, int responses
		, std::shared_ptr<put_ctx> ctx
		, std::function<void(dht::item const&, int)> cb);

	void send_callback(entry const& it
		, std::vector<std::pair<dht::node_entry, bool>> const& success_nodes
		, std::shared_ptr<relay_ctx> ctx
		, std::function<void(entry const&
			, std::vector<std::pair<dht::node_entry, bool>> const&)> cb);

	void register_relay_listener(std::shared_ptr<relay_listener> listener)
	{
		m_relay_listeners.insert(std::move(listener));
	}

	void unregister_relay_listener(std::shared_ptr<relay_listener> listener)
	{
		m_relay_listeners.erase(std::move(listener));
	}

	void invoking_timeout(error_code const& e);

private:

	bool m_running;

	io_context& m_ios;
	aux::session_interface& m_session;
	aux::session_settings const& m_settings;
	counters& m_counters;

	congestion_controller m_congestion_controller;

	std::set<std::shared_ptr<relay_listener>> m_relay_listeners;

	std::queue<rpc> m_rpc_queue;

	aux::deadline_timer m_invoking_timer;
};

} // namespace transport
} // namespace ip2

#endif // IP2_TRANSPORT_TRANSPORTER_HPP
