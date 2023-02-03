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

void put_context::add_invoked_hash(sha1_hash const& h, bool seg)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(h, hex_hash);
#endif

	m_flying_segments.insert(h);

	auto it = m_invoked_hashes.find(h);
	if (it == m_invoked_hashes.end())
	{
		m_invoked_hashes.insert(std::pair<sha1_hash, int>(h, 1));
		if (seg)
		{
#ifndef TORRENT_DISABLE_LOGGING
			m_logger.log(aux::LOG_INFO, "[%u] put segment:%s, times:%d"
				, id(), hex_hash, 1);
#endif
		}
		else
		{
#ifndef TORRENT_DISABLE_LOGGING
			m_logger.log(aux::LOG_INFO, "[%u] put index:%s, times:%d"
				, id(), hex_hash, 1);
#endif
		}

		return;
	}

	it->second++;

	if (seg)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_INFO, "[%u] put segment:%s, times:%d"
			, id(), hex_hash, it->second);
#endif
	}
	else
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_INFO, "[%u] put index:%s, times:%d"
			, id(), hex_hash, it->second);
#endif
	}
}

void put_context::add_callbacked_hash(sha1_hash const& h, int response, bool seg)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(h, hex_hash);
#endif

	m_flying_segments.erase(h);

	auto it = m_invoked_hashes.find(h);
	if (it == m_invoked_hashes.end())
	{
		m_callbacked_hashes.insert(std::pair<sha1_hash, int>(h, response));
		if (seg)
		{
#ifndef TORRENT_DISABLE_LOGGING
			m_logger.log(aux::LOG_INFO, "[%u] put segment callback:%s, responses:%d"
				, id(), hex_hash, response);
#endif
		}
		else
		{
#ifndef TORRENT_DISABLE_LOGGING
			m_logger.log(aux::LOG_INFO, "[%u] put index callback:%s, responses:%d"
				, id(), hex_hash, response);
#endif
		}

		return;
	}

	it->second = response;

	if (seg)
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_INFO, "[%u] put segment callback:%s, responses:%d"
			, id(), hex_hash, it->second);
#endif
	}
	else
	{
#ifndef TORRENT_DISABLE_LOGGING
		m_logger.log(aux::LOG_INFO, "[%u] put index callback:%s, responses:%d"
			, id(), hex_hash, it->second);
#endif
	}
}

bool put_context::is_reput_allowed(sha1_hash const& h)
{
#ifndef TORRENT_DISABLE_LOGGING
	char hex_hash[41];
	aux::to_hex(h, hex_hash);
#endif

	auto it = m_invoked_hashes.find(h);

	if (it == m_invoked_hashes.end())
	{
		return true;
	}

	int times = it->second;
#ifndef TORRENT_DISABLE_LOGGING
	m_logger.log(aux::LOG_INFO, "[%u] allowed reput hash:%s, times:%d"
		, id(), hex_hash, times);
#endif

	return times < reput_times_limit;
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
