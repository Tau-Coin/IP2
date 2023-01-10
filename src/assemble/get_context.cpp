/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/get_context.hpp"
#include "ip2/assemble/protocol.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include <ip2/hex.hpp> // to_hex
#endif

using namespace ip2::assemble::protocol;

namespace ip2 {
namespace assemble {

get_context::get_context(assemble_logger& logger, dht::public_key const& sender
	, aux::uri const& blob_uri, dht::timestamp ts)
	: m_logger(logger)
	, m_sender(sender)
	, m_uri(blob_uri)
	, m_ts(ts.value)
	, m_uri_hash(blob_uri.bytes.data())
{}

void get_context::start_getting_seg(sha1_hash const& h)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(h, hex_hash);
#endif

	m_flying_segments.insert(h);

	auto it = m_invoked_seg_hashes.find(h);

	if (it == m_invoked_seg_hashes.end())
	{
		m_invoked_seg_hashes.insert(std::pair<sha1_hash, int>(h, 1));
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_INFO, "[%u] start getting segment:%s, times:%d"
			, id(), hex_hash, 1);
#endif

		return;
	}

	it->second++;

#ifndef TORRENT_DISABLE_LOGGING
	m_logger.log(aux::LOG_INFO, "[%u] start getting segment:%s, times:%d"
		, id(), hex_hash, it->second);
#endif
}

bool get_context::is_getting_allowed(sha1_hash const& h)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(h, hex_hash);
#endif

	auto it = m_invoked_seg_hashes.find(h);

	if (it == m_invoked_seg_hashes.end())
	{
        return true;
	}

    int times = it->second;

#ifndef TORRENT_DISABLE_LOGGING 
	m_logger.log(aux::LOG_INFO, "[%u] allowed getting segment:%s, times:%d"
		, id(), hex_hash, times);
#endif

	return times <= reget_times_limit;
}

api::error_code get_context::on_root_index_got(dht::item const& it)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_uri[41];
	aux::to_hex(m_uri_hash, hex_uri);
#endif

	entry const& proto = it.value();
	protocol::basic_protocol bp;
	api::error_code err;

	std::tie(bp, err) = protocol::construct_protocol(proto);
	if (err != api::NO_ERROR)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR, "[%u] parse index[%s] error: %d"
			, id(), hex_uri, err);
#endif

		return err;
	}

	if (bp.get_name() != protocol::blob_index_protocol::name)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_ERR, "[%u] parse index error:%s, name:%s"
			, id(), hex_uri, bp.get_name().c_str());
#endif

		return api::ASSEMBLE_NAME_ERROR;
	}

	protocol::blob_index_protocol *index_proto
		= static_cast<protocol::blob_index_protocol*>(&bp);
	index_proto->seg_hashes(m_root_index);

	return api::NO_ERROR;
}

api::error_code get_context::on_segment_got(dht::item const& it
	, sha1_hash const& seg_hash)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(seg_hash, hex_hash);
#endif

	entry const& proto = it.value();
	protocol::basic_protocol bp;
	api::error_code err;

	std::tie(bp, err) = protocol::construct_protocol(proto);
	if (err != api::NO_ERROR)
	{
#ifndef TORRENT_DISABLE_LOGGING 
		m_logger.log(aux::LOG_ERR, "[%u] parse segment[%s] error:%d"
			, id(), hex_hash, err);
#endif

		return err;
	}

	if (bp.get_name() != protocol::blob_seg_protocol::name)
	{
#ifndef TORRENT_DISABLE_LOGGING 
		m_logger.log(aux::LOG_ERR, "[%u] parse segment error:%s, name:%s"
			, id(), hex_hash, bp.get_name().c_str());
#endif

		return api::ASSEMBLE_NAME_ERROR;
	}

	protocol::blob_seg_protocol *seg_proto
		= static_cast<protocol::blob_seg_protocol*>(&bp);
	std::string value = seg_proto->seg_value();

	m_segments.insert(std::pair<sha1_hash, std::string>(seg_hash, std::move(value)));

	return api::NO_ERROR;
}

void get_context::done()
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_sender[65];
	char hex_uri[41];
	aux::to_hex(m_sender.bytes, hex_sender);
	aux::to_hex(m_uri.bytes, hex_uri);

	m_logger.log(aux::LOG_INFO
		, "[%u] get DONE: sender: %s, uri:%s, err:%d, invoked:%d, index:%d, value size:%d"
		, id(), hex_sender, hex_uri, get_error()
		, (int)m_invoked_seg_hashes.size()
		, (int)m_root_index.size()
		, (int)m_segments.size());
#endif
}

} // namespace assemble
} // namespace ip2
