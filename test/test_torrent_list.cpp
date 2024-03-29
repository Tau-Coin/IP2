/*

Copyright (c) 2018, Steven Siloti
Copyright (c) 2019-2020, Arvid Norberg
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "test.hpp"
#include "ip2/aux_/torrent_list.hpp"
#include "ip2/sha1_hash.hpp"

using namespace lt;

namespace {

sha1_hash const sha1_1("abababababababababab");
sha1_hash const sha1_2("cbcbcbcbcbcbcbcbcbcb");
sha1_hash const sha1_3("cdcdcdcdcdcdcdcdcdcd");
sha1_hash const sha1_4("edededededededededed");
sha256_hash const sha2_1("xbxbxbxbxbxbxbxbxbxbxbxbxbxbxbxb");
sha1_hash const sha2_1_truncated("xbxbxbxbxbxbxbxbxbxb");

info_hash_t const v1(sha1_1);
info_hash_t const v2(sha2_1);
info_hash_t const hybrid(sha1_1, sha2_1);

using ih = lt::info_hash_t;
}

TORRENT_TEST(torrent_list_empty)
{
	aux::torrent_list<int> l;
	TEST_CHECK(l.empty());
	TEST_CHECK(l.begin() == l.end());
	l.insert(v1, std::make_shared<int>(1337));
	TEST_CHECK(!l.empty());
	TEST_CHECK(l.begin() != l.end());
}

TORRENT_TEST(torrent_list_size)
{
	aux::torrent_list<int> l;
	TEST_EQUAL(l.size(), 0);
	l.insert(ih(sha1_1), std::make_shared<int>(1337));
	TEST_EQUAL(l.size(), 1);
	l.insert(ih(sha1_2), std::make_shared<int>(1338));
	TEST_EQUAL(l.size(), 2);
	l.insert(ih(sha1_3), std::make_shared<int>(1339));
	TEST_EQUAL(l.size(), 3);

	TEST_EQUAL(*l.find(sha1_1), 1337);
	TEST_EQUAL(*l.find(sha1_2), 1338);
	TEST_EQUAL(*l.find(sha1_3), 1339);
}

TORRENT_TEST(torrent_list_duplicates)
{
	aux::torrent_list<int> l;
	TEST_EQUAL(l.size(), 0);
	TEST_CHECK(l.insert(v1, std::make_shared<int>(1337)));
	TEST_EQUAL(l.size(), 1);
	TEST_CHECK(!l.insert(v1, std::make_shared<int>(1338)));
	TEST_EQUAL(l.size(), 1);
	TEST_EQUAL(*l.find(sha1_1), 1337);
}

TORRENT_TEST(torrent_list_duplicates_v1)
{
	aux::torrent_list<int> l;
	TEST_EQUAL(l.size(), 0);
	TEST_CHECK(l.insert(hybrid, std::make_shared<int>(1337)));
	TEST_EQUAL(l.size(), 1);
	TEST_CHECK(!l.insert(v1, std::make_shared<int>(1338)));
	TEST_EQUAL(l.size(), 1);
	TEST_EQUAL(*l.find(sha1_1), 1337);
	TEST_EQUAL(*l.find(sha2_1_truncated), 1337);
}

TORRENT_TEST(torrent_list_duplicates_v2)
{
	aux::torrent_list<int> l;
	TEST_EQUAL(l.size(), 0);
	TEST_CHECK(l.insert(hybrid, std::make_shared<int>(1337)));
	TEST_EQUAL(l.size(), 1);
	TEST_CHECK(!l.insert(v2, std::make_shared<int>(1338)));
	TEST_EQUAL(l.size(), 1);
	TEST_EQUAL(*l.find(sha1_1), 1337);
	TEST_EQUAL(*l.find(sha2_1_truncated), 1337);
}

TORRENT_TEST(torrent_list_duplicates_self)
{
	aux::torrent_list<int> l;
	TEST_EQUAL(l.size(), 0);
	TEST_CHECK(l.insert(ih(sha2_1_truncated, sha2_1), std::make_shared<int>(1337)));
	TEST_EQUAL(l.size(), 1);
	TEST_CHECK(*l.find(sha2_1_truncated) == 1337);

	TEST_CHECK(l.erase(ih(sha2_1_truncated, sha2_1)));
	TEST_EQUAL(l.size(), 0);
	TEST_CHECK(l.find(sha2_1_truncated) == nullptr);
}

TORRENT_TEST(torrent_truncated_list_lookup)
{
	aux::torrent_list<int> l;
	l.insert(v2, std::make_shared<int>(1337));
	l.insert(v1, std::make_shared<int>(1338));

	TEST_EQUAL(*l.find(sha2_1_truncated), 1337);
	TEST_EQUAL(*l.find(sha1_1), 1338);
	TEST_CHECK(l.find(sha1_3) == nullptr);
}

TORRENT_TEST(torrent_list_lookup)
{
	aux::torrent_list<int> l;
	l.insert(ih(sha1_1), std::make_shared<int>(1337));
	l.insert(ih(sha1_2), std::make_shared<int>(1338));

	TEST_EQUAL(*l.find(sha1_1), 1337);
	TEST_EQUAL(*l.find(sha1_2), 1338);
	TEST_CHECK(l.find(sha1_3) == nullptr);
}

TORRENT_TEST(torrent_list_order)
{
	aux::torrent_list<int> l;
	l.insert(ih(sha1_1), std::make_shared<int>(1));
	l.insert(ih(sha1_2), std::make_shared<int>(2));
	l.insert(ih(sha1_3), std::make_shared<int>(3));
	l.insert(ih(sha1_4), std::make_shared<int>(0));

	// iteration order is the same as insertion order, not sort order of
	// info-hashes
	std::vector<int> order;
	for (auto i : l)
	{
		order.push_back(*i);
	}

	TEST_CHECK((order == std::vector<int>{1, 2, 3, 0}));

	TEST_EQUAL(*l[0], 1);
	TEST_EQUAL(*l[1], 2);
	TEST_EQUAL(*l[2], 3);
	TEST_EQUAL(*l[3], 0);
}

TORRENT_TEST(torrent_list_erase)
{
	aux::torrent_list<int> l;
	l.insert(v1, std::make_shared<int>(1337));
	TEST_CHECK(!l.empty());

	// this doesn't exist, returns false
	TEST_CHECK(!l.erase(ih(sha1_2)));
	TEST_CHECK(!l.empty());

	TEST_EQUAL(*l.find(sha1_1), 1337);
	TEST_CHECK(l.erase(ih(sha1_1)));
	TEST_CHECK(l.find(sha1_1) == nullptr);
	TEST_CHECK(l.empty());
}

TORRENT_TEST(torrent_list_erase2)
{
	aux::torrent_list<int> l;
	l.insert(ih(sha1_1), std::make_shared<int>(1337));
	l.insert(ih(sha1_2), std::make_shared<int>(1338));

	TEST_EQUAL(*l.find(sha1_1), 1337);
	TEST_EQUAL(l.size(), 2);
	TEST_CHECK(!l.empty());

	// delete an entry that isn't the last one
	TEST_CHECK(l.erase(ih(sha1_1)));
	TEST_CHECK(l.find(sha1_1) == nullptr);
	TEST_EQUAL(l.size(), 1);
	TEST_CHECK(!l.empty());
	TEST_EQUAL(*l.find(sha1_2), 1338);
}

TORRENT_TEST(torrent_list_clear)
{
	aux::torrent_list<int> l;
	l.insert(ih(sha1_1), std::make_shared<int>(1));
	l.insert(ih(sha1_2), std::make_shared<int>(2));
	l.insert(ih(sha1_3), std::make_shared<int>(3));
	l.insert(ih(sha1_4), std::make_shared<int>(0));

	TEST_CHECK(!l.empty());

	TEST_CHECK(*l.find(sha1_1) == 1);
	TEST_CHECK(*l.find(sha1_2) == 2);
	TEST_CHECK(*l.find(sha1_3) == 3);
	TEST_CHECK(*l.find(sha1_4) == 0);

	l.clear();
	TEST_CHECK(l.empty());

	TEST_CHECK(l.find(sha1_1) == nullptr);
	TEST_CHECK(l.find(sha1_2) == nullptr);
	TEST_CHECK(l.find(sha1_3) == nullptr);
	TEST_CHECK(l.find(sha1_4) == nullptr);
}

#if !defined TORRENT_DISABLE_ENCRYPTION
TORRENT_TEST(torrent_list_obfuscated_lookup)
{
	aux::torrent_list<int> l;
	l.insert(ih(sha1_1), std::make_shared<int>(1337));

	TEST_EQUAL(*l.find(sha1_1), 1337);
	static char const req2[4] = {'r', 'e', 'q', '2'};
	hasher h(req2);
	h.update(sha1_1);
	TEST_EQUAL(*l.find_obfuscated(h.final()), 1337);
	// this should not exist as an obfuscated hash
	TEST_CHECK(l.find_obfuscated(sha1_1) == nullptr);
}
#endif

