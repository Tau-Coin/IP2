/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_RELAY_CONTEXT_HPP
#define IP2_ASSEMBLE_RELAY_CONTEXT_HPP

#include "ip2/assemble/context.hpp"
#include "ip2/assemble/assemble_logger.hpp"

#include <ip2/kademlia/item.hpp>
#include <ip2/kademlia/node_id.hpp>
#include <ip2/kademlia/types.hpp>

#include <ip2/api/error_code.hpp>
#include <ip2/sha1_hash.hpp>
#include <ip2/uri.hpp>

using namespace ip2::api;

namespace ip2 {
namespace assemble {

enum relay_type
{
	URI,
	MESSAGE,
};

struct TORRENT_EXTRA_EXPORT relay_context final : context
{
public:

	relay_context(assemble_logger& logger, dht::public_key const& receiver
		, sha256_hash const& msg_id, relay_type t);

	dht::public_key get_receiver() { return m_receiver; }

	sha256_hash get_msg_id() { return m_msg_id; }

	relay_type get_relay_type() { return m_type; } 

	void start_relay();
	void done() override;

private:

	assemble_logger& m_logger;

	dht::public_key m_receiver;
	sha256_hash m_msg_id;

	relay_type m_type;
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_RELAY_CONTEXT_HPP
