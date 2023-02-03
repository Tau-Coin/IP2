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
	, dht::public_key const& receiver)
	: m_logger(logger)
	, m_receiver(receiver)
	, m_type(relay_type::MESSAGE)
{}

relay_context::relay_context(assemble_logger& logger
	, dht::public_key const& receiver
	, aux::uri const& data_uri
	, dht::timestamp ts)
	: m_logger(logger)
	, m_receiver(receiver)
	, m_uri(data_uri)
	, m_ts(ts)
	, m_type(relay_type::URI)
{}

void relay_context::start_relay()
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_receiver[65];
	aux::to_hex(m_receiver.bytes, hex_receiver);
#endif

	if (m_type == relay_type::MESSAGE)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_INFO
			, "[%u] start relay message to %s", id(), hex_receiver);
#endif
	}
	else
	{
#ifndef TORRENT_DISABLE_LOGGING
		char hex_uri[41];
		aux::to_hex(m_uri.bytes, hex_uri);
		m_logger.log(aux::LOG_INFO
			, "[%u] start relay uri %s to %s", id(), hex_uri, hex_receiver);
#endif
	}
}

void relay_context::done()
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_receiver[65];
	aux::to_hex(m_receiver.bytes, hex_receiver);

	m_logger.log(aux::LOG_INFO
		, "[%u] relay DONE: receiver: %s, err: %d", id(), hex_receiver, get_error());
#endif
}

} // namespace assemble
} // namespace ip2
