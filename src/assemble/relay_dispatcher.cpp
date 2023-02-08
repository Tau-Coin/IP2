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

relay_dispatcher::relay_dispatcher(getter& g, relayer& r, assemble_logger& logger)
	: m_getter(g)
	, m_relayer(r)
	, m_logger(logger)
{}

void relay_dispatcher::on_dht_relay(dht::public_key const& from, entry const& payload)
{
	std::shared_ptr<protocol::basic_protocol> bp;
	api::error_code err;
	std::tie(bp, err) = protocol::construct_protocol(payload, m_logger);
	if (err != api::NO_ERROR)
	{
#ifndef TORRENT_DISABLE_LOGGING 
		m_logger.log(aux::LOG_ERR, "parse protocol error:%d", err);
#endif

		return;
    }

	if (strncmp(bp->get_name().c_str(), protocol::relay_uri_protocol::name.c_str()
			, 1) == 0)
	{
		std::shared_ptr<protocol::relay_uri_protocol> rup
			= std::dynamic_pointer_cast<protocol::relay_uri_protocol>(bp);

		m_getter.on_incoming_relay_uri(rup->pk()
			, rup->blob_uri(), rup->ts());
	}
	else if (strncmp(bp->get_name().c_str(), protocol::relay_msg_protocol::name.c_str()
			, 1) == 0)
	{
		std::shared_ptr<protocol::relay_msg_protocol> rmp
			= std::dynamic_pointer_cast<protocol::relay_msg_protocol>(bp);

		m_relayer.on_incoming_relay_message(from, rmp->msg());
	}
	else
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR, "parse protocol unkown name: %s", bp->get_name().c_str());
#endif
	}
}

} // namespace assemble
} // namespace ip2
