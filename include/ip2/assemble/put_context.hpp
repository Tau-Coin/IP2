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

	void add_root_index(sha1_hash const& h);

	void get_root_index(std::vector<sha1_hash>& ri)
	{
		for (auto& i : m_root_index)
		{
			ri.push_back(i);
		}
	}

	void add_invoked_hash(sha1_hash const& h);

	void add_callbacked_hash(sha1_hash const& h, int response);

	bool is_done()
	{
		return m_invoked_hashes.size() == m_callbacked_hashes.size();
	}

	void done() override;

private:

	assemble_logger& m_logger;

	dht::public_key m_sender;
	aux::uri m_uri;

	std::uint32_t m_seg_count = 0;

	std::vector<sha1_hash> m_root_index;

	// segment hash or index(root or children) hash
	std::vector<sha1_hash> m_invoked_hashes;

	std::map<sha1_hash, int> m_callbacked_hashes; // put reponses
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_PUT_CONTEXT_HPP
