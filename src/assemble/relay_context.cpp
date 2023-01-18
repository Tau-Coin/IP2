/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/relay_context.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include <ip2/hex.hpp> // to_hex
#endif

namespace ip2 {
namespace assemble {

relay_context::relay_context(assemble_logger& logger
	, dht::public_key const& receiver, sha1_hash const& msg_id, relay_type t)
	: m_logger(logger)
	, m_receiver(receiver)
	, m_msg_id(msg_id)
	, m_type(t)
{}

void relay_context::start_relay()
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_receiver[65];
	aux::to_hex(m_receiver.bytes, hex_receiver);

	m_logger.log(aux::LOG_INFO
		, "[%u] start relay: receiver: %s, msg_id: %s"
		, id(), hex_receiver, aux::to_hex(m_msg_id).c_str());
#endif
}

void relay_context::done()
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_receiver[65];
	aux::to_hex(m_receiver.bytes, hex_receiver);

	m_logger.log(aux::LOG_INFO
		, "[%u] relay DONE: receiver: %s, msg_id: %s, err: %d"
		, id(), hex_receiver, aux::to_hex(m_msg_id).c_str(), get_error());
#endif
}

} // namespace assemble
} // namespace ip2
