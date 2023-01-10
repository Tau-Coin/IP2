/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/put_context.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include <ip2/hex.hpp> // to_hex
#endif

namespace ip2 {
namespace assemble {

put_context::put_context(assemble_logger& logger, dht::public_key const& sender
	, aux::uri const& blob_uri, std::uint32_t seg_count)
	: context()
	, m_logger(logger)
	, m_sender(sender)
	, m_uri(blob_uri)
	, m_seg_count(seg_count)
{}

void put_context::add_root_index(sha1_hash const& h)
{
	m_root_index.push_back(h);

#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(h, hex_hash);

	m_logger.log(aux::LOG_INFO, "[%u] add root index:%s", id(), hex_hash);
#endif
}

void put_context::add_invoked_hash(sha1_hash const& h)
{
	m_invoked_hashes.push_back(h);

#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(h, hex_hash);

	m_logger.log(aux::LOG_INFO, "[%u] put index or segment:%s", id(), hex_hash);
#endif
}

void put_context::add_callbacked_hash(sha1_hash const& h, int response)
{
	m_callbacked_hashes.insert(std::pair<sha1_hash, int>(h, response));

#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(h, hex_hash);

    m_logger.log(aux::LOG_INFO, "[%u] put index or segment callback:%s, responses:%d"
		, id(), hex_hash, response);
#endif
}

void put_context::done()
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_uri[41];
	aux::to_hex(m_uri.bytes, hex_uri);

	m_logger.log(aux::LOG_INFO
		, "[%u] put DONE: uri:%s, err:%d, seg_count:%u, invoked:%d, cb:%d"
		, id(), hex_uri, get_error(), m_seg_count
		, (int)m_invoked_hashes.size(), (int)m_callbacked_hashes.size());
#endif
}

} // namespace assemble
} // namespace ip2
