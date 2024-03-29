/*

Copyright (c) 2006-2008, 2010-2016, 2018-2020, Arvid Norberg
Copyright (c) 2016, Steven Siloti
Copyright (c) 2016, 2018, 2020-2021, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include <algorithm>

#include "ip2/kademlia/node_id.hpp"
#include "ip2/kademlia/node_entry.hpp"
#include "ip2/kademlia/ed25519.hpp"
#include "ip2/assert.hpp"
#include "ip2/aux_/ip_helpers.hpp" // for is_local et.al
#include "ip2/aux_/random.hpp" // for random
#include "ip2/hasher.hpp" // for hasher
#include "ip2/hex.hpp" // for hex
#include "ip2/aux_/crc32c.hpp" // for crc32c

namespace ip2::dht {

// returns the distance between the two nodes
// using the kademlia XOR-metric
node_id distance(node_id const& n1, node_id const& n2)
{
	return n1 ^ n2;
}

// returns true if: distance(n1, ref) < distance(n2, ref)
bool compare_ref(node_id const& n1, node_id const& n2, node_id const& ref)
{
	node_id const lhs = n1 ^ ref;
	node_id const rhs = n2 ^ ref;
	return lhs < rhs;
}

// returns n in: 2^n <= distance(n1, n2) < 2^(n+1)
// useful for finding out which bucket a node belongs to
int distance_exp(node_id const& n1, node_id const& n2)
{
	// TODO: it's a little bit weird to return 255 - leading zeroes. It should
	// probably be 256 - leading zeroes, but all other code in here is tuned to
	// this expectation now, and it doesn't really matter (other than complexity)
	return std::max(255 - distance(n1, n2).count_leading_zeroes(), 0);
}

int min_distance_exp(node_id const& n1, std::vector<node_id> const& ids)
{
	TORRENT_ASSERT(ids.size() > 0);

	int min = 256; // see distance_exp for the why of this constant
	for (auto const& node_id : ids)
	{
		min = std::min(min, distance_exp(n1, node_id));
	}

	return min;
}

static std::uint32_t secret = 0;

void make_id_secret(node_id& in)
{
	if (secret == 0) secret = aux::random(0xfffffffe) + 1;

	std::uint32_t const rand = aux::random(0xffffffff);

	// generate the last 4 bytes as a "signature" of the previous 4 bytes. This
	// lets us verify whether a hash came from this function or not in the future.
	hasher h(reinterpret_cast<char const*>(&secret), 4);
	h.update(reinterpret_cast<char const*>(&rand), 4);
	sha1_hash const secret_hash = h.final();
	std::memcpy(&in[32 - 12], &secret_hash[0], 12);
	std::memcpy(&in[32 - 16], &rand, 4);
}

node_id generate_random_id()
{
	public_key pk;
	std::tie(pk, std::ignore) = ed25519_create_keypair(ed25519_create_seed());

	return node_id(span<char const>(pk.bytes));
}

node_id get_node_id(ip2::aux::session_settings const& settings)
{
	std::array<char, 32> seed;
	public_key pk;

	const char* account_seed = settings.get_str(ip2::settings_pack::account_seed).c_str();
	span<char const> hexseed(account_seed, 64);
	ip2::aux::from_hex(hexseed, seed.data());
	std::tie(pk, std::ignore) = ed25519_create_keypair(seed);

	return node_id(span<char const>(pk.bytes));
}

node_id generate_secret_id()
{
	node_id ret = generate_random_id();
	make_id_secret(ret);
	return ret;
}

bool verify_secret_id(node_id const& nid)
{
	if (secret == 0) return false;

	hasher h(reinterpret_cast<char*>(&secret), 4);
	h.update(reinterpret_cast<char const*>(&nid[32 - 16]), 4);
	sha1_hash secret_hash = h.final();
	return std::memcmp(&nid[32 - 12], &secret_hash[0], 12) == 0;
}

node_id generate_prefix_mask(int const bits)
{
	TORRENT_ASSERT(bits >= 0);
	TORRENT_ASSERT(bits <= 256);
	node_id mask;
	std::size_t b = 0;
	for (; int(b) < bits - 7; b += 8) mask[b / 8] |= 0xff;
	if (bits < 256) mask[b / 8] |= (0xff << (8 - (bits & 7))) & 0xff;
	return mask;
}

} // namespace ip2::dht
