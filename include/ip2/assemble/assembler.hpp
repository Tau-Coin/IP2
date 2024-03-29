/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_ASSEMBLER_HPP
#define IP2_ASSEMBLE_ASSEMBLER_HPP

#include "ip2/assemble/assemble_logger.hpp"
#include "ip2/assemble/getter.hpp"
#include "ip2/assemble/putter.hpp"
#include "ip2/assemble/relayer.hpp"
#include "ip2/assemble/relay_dispatcher.hpp"

#include <ip2/entry.hpp>
#include <ip2/io_context.hpp>
#include "ip2/api/error_code.hpp"
#include "ip2/aux_/common.h"
#include "ip2/aux_/deadline_timer.hpp"
#include "ip2/span.hpp"
#include "ip2/uri.hpp"

#include <ip2/kademlia/types.hpp>

#include <string>
#include <tuple>

using namespace ip2::api;

namespace ip2 {

	struct counters;

namespace aux {
    struct session_settings;
	struct session_interface;
}

namespace assemble {

class TORRENT_EXTRA_EXPORT assembler final
	: public std::enable_shared_from_this<assembler>, public assemble_logger
{

public:

	assembler(io_context& ios
		, aux::session_interface& session
		, aux::session_settings const& settings
		, counters& cnt);

	assembler(assembler const&) = delete;
	assembler& operator=(assembler const&) = delete;
	assembler(assembler&&) = delete;
	assembler& operator=(assembler&&) = delete;

	~assembler();

	std::shared_ptr<assembler> self() { return shared_from_this(); }

	bool should_log(aux::LOG_LEVEL log_level) const override;
	void log(aux::LOG_LEVEL log_level, char const* fmt, ...)
		noexcept override TORRENT_FORMAT(3,4);

	void start();
	void stop();

	void update_node_id();

	api::error_code put(span<char const> blob, aux::uri const& blob_uri);

	api::error_code get(dht::public_key const& sender
		, aux::uri data_uri, dht::timestamp ts);

	api::error_code relay_message(dht::public_key const& receiver
		, span<char const> message);

	api::error_code relay_uri(dht::public_key const& receiver
		, aux::uri const& data_uri, dht::timestamp ts);

private:

	io_context& m_ios;
	aux::session_interface& m_session;
	aux::session_settings const& m_settings;
	counters& m_counters;

	dht::public_key m_self_pubkey;

	getter m_getter;
	putter m_putter;
	relayer m_relayer;

	std::shared_ptr<relay_dispatcher> m_relay_dispatcher;

	bool m_running = false;
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_ASSEMBLER_HPP
