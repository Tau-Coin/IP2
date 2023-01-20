/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_GET_CONTEXT_HPP
#define IP2_ASSEMBLE_GET_CONTEXT_HPP

#include "ip2/assemble/context.hpp"
#include "ip2/assemble/assemble_logger.hpp"

#include <ip2/kademlia/item.hpp>
#include <ip2/kademlia/node_id.hpp>
#include <ip2/kademlia/types.hpp>

#include <ip2/api/error_code.hpp>
#include <ip2/sha1_hash.hpp>
#include <ip2/uri.hpp>

#include <map>
#include <set>
#include <tuple>
#include <vector>

using namespace ip2::api;

namespace ip2 {
namespace assemble {

static constexpr int reget_times_limit = 3;

struct TORRENT_EXTRA_EXPORT get_context final : context
{
public:

	get_context(assemble_logger& logger, dht::public_key const& sender
		, aux::uri const& blob_uri, dht::timestamp ts);

	dht::public_key get_sender() { return m_sender; }

	aux::uri get_uri() { return m_uri; }

	std::int64_t get_timestamp() { return m_ts.value; }

	bool is_root_index(sha1_hash const& h) { return h == m_uri_hash; }

	void get_root_index(std::vector<sha1_hash>& index)
	{
		for (auto& i : m_root_index)
		{
			index.push_back(i);
		}
	}

	void start_getting_seg(sha1_hash const& h);

	bool is_getting_allowed(sha1_hash const& h);

	void on_arrived(sha1_hash const& hash)
	{
		m_flying_segments.erase(hash);
	}

	api::error_code on_root_index_got(dht::item const& it);

	api::error_code on_segment_got(dht::item const& it, sha1_hash const& seg_hash);

	void done() override;

	bool is_done()
	{
		return m_flying_segments.size() == 0;
	}

	bool get_segments_blob(std::string& value)
	{
		// ignore broken blob
		if (m_root_index.size() != m_segments.size()) return false;

		for (auto& i : m_root_index)
		{
			auto it = m_segments.find(i);
			if (it == m_segments.end()) return false;

			value.append(it->second, it->second.size());
		}

		return true;
	}

private:

	assemble_logger& m_logger;

	dht::public_key m_sender;
	aux::uri m_uri;
	dht::timestamp m_ts;

	sha1_hash m_uri_hash;

	// 'int' field represents the count of getting segment.
	std::map<sha1_hash, int> m_invoked_seg_hashes;

	std::set<sha1_hash> m_flying_segments;

	std::vector<sha1_hash> m_root_index;
	std::map<sha1_hash, std::string> m_segments;
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_GET_CONTEXT_HPP
