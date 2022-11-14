/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_PUT_CONTEXT_HPP
#define IP2_ASSEMBLE_PUT_CONTEXT_HPP

#include "ip2/assemble/context.hpp"
#include "ip2/assemble/assemble_logger.hpp"

#include <ip2/kademlia/node_id.hpp>
#include <ip2/kademlia/types.hpp>

#include <ip2/sha1_hash.hpp>
#include <ip2/uri.hpp>

#include <map>
#include <vector>

namespace ip2 {
namespace assemble {

struct TORRENT_EXTRA_EXPORT put_context final : context
{
public:

	put_context(assemble_logger& logger, dht::public_key const& sender
		, aux::uri const& blob_uri, std::uint32_t seg_count);

	void add_invoked_segment_hash(sha1_hash const& h);

	void add_callbacked_segment_hash(sha1_hash const& h, int response);

	sha1_hash get_top_hash();

	bool is_done()
	{
		return m_invoked_seg_hashes.size() == m_callbacked_seg_hashes.size();
	}

	void done() override;

private:

	assemble_logger& m_logger;

	dht::public_key m_sender;
	aux::uri m_uri;

	std::uint32_t m_seg_count = 0;

	std::vector<sha1_hash> m_invoked_seg_hashes; // segment hash

	std::map<sha1_hash, int> m_callbacked_seg_hashes; // segment put reponses
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_PUT_CONTEXT_HPP
