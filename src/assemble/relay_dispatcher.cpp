/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/relay_dispatcher.hpp"
#include "ip2/assemble/protocol.hpp"

using namespace ip2::assemble::protocol;

namespace ip2 {
namespace assemble {

relay_dispatcher::relay_dispatcher(assemble_logger& logger
	, getter& g, relayer& r)
	: m_logger(logger)
	, m_getter(g)
	, m_relayer(r)
{}

void relay_dispatcher::on_dht_relay(sha256_hash const& from, entry const& payload)
{
	protocol::basic_protocol bp; 
	api::error_code err;

	std::tie(bp, err) = protocol::construct_protocol(payload);
	if (err != api::NO_ERROR)
	{
#ifndef TORRENT_DISABLE_LOGGING 
		m_logger.log(aux::LOG_ERR, "parse protocol error:%d", err);
#endif

		return;
    }

	if (bp.get_name() == protocol::relay_uri_protocol::name)
	{
		protocol::relay_uri_protocol *rup
			= static_cast<protocol::relay_uri_protocol*>(&bp);

		m_getter.on_incoming_relay_request(rup->pk()
			, rup->blob_uri(), rup->ts());
	}
	else if (bp.get_name() == protocol::relay_msg_protocol::name)
	{
		protocol::relay_msg_protocol *rmp
			= static_cast<protocol::relay_msg_protocol*>(&bp);

		dht::public_key pk(from.data());
		m_relayer.on_incoming_relay_message(pk, rmp->msg());
	}
	else
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR, "parse protocol unkown name: %s", bp.get_name().c_str());
#endif
	}
}

} // namespace assemble
} // namespace ip2
