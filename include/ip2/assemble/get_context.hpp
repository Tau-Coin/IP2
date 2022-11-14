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

	std::int64_t get_timestamp() { return m_ts.value; }

	void start_getting_seg(sha1_hash const& h);

	bool is_getting_allowed(sha1_hash const& h);

	// return tuple<next_seg_hash, api:error_code>
	std::tuple<sha1_hash, api::error_code> on_item_got(dht::item const& it
		, sha1_hash seg_hash);

	void done() override;

private:

	assemble_logger& m_logger;

	dht::public_key m_sender;
	aux::uri m_uri;
	dht::timestamp m_ts;

	// 'int' field represents try to getting count.
	std::map<sha1_hash, int> m_invoked_seg_hashes;

	std::string m_value; // append blob segment
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_GET_CONTEXT_HPP
