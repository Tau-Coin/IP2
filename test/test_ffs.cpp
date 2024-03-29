/*

Copyright (c) 2016-2017, 2019-2020, Arvid Norberg
Copyright (c) 2016, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "test.hpp"
#include "ip2/span.hpp"
#include "ip2/hex.hpp" // from_hex
#include "ip2/aux_/ffs.hpp"
#include "ip2/aux_/byteswap.hpp"

using namespace lt;

static void to_binary(char const* s, std::uint32_t* buf)
{
	aux::from_hex({s, 40}, reinterpret_cast<char*>(&buf[0]));
}

TORRENT_TEST(count_leading_zeros)
{
	std::vector<std::pair<char const*, int>> const tests = {
		{ "ffffffffffffffffffffffffffffffffffffffff", 0 },
		{ "0000000000000000000000000000000000000000", 160 },
		{ "fff0000000000000000000000000000000000000", 0 },
		{ "7ff0000000000000000000000000000000000000", 1 },
		{ "3ff0000000000000000000000000000000000000", 2 },
		{ "1ff0000000000000000000000000000000000000", 3 },
		{ "0ff0000000000000000000000000000000000000", 4 },
		{ "07f0000000000000000000000000000000000000", 5 },
		{ "03f0000000000000000000000000000000000000", 6 },
		{ "01f0000000000000000000000000000000000000", 7 },
		{ "00f0000000000000000000000000000000000000", 8 },
		{ "0070000000000000000000000000000000000000", 9 },
		{ "0030000000000000000000000000000000000000", 10 },
		{ "0010000000000000000000000000000000000000", 11 },
		{ "0000000ffff00000000000000000000000000000", 28 },
		{ "00000007fff00000000000000000000000000000", 29 },
		{ "00000003fff00000000000000000000000000000", 30 },
		{ "00000001fff00000000000000000000000000000", 31 },
		{ "00000000fff00000000000000000000000000000", 32 },
		{ "000000007ff00000000000000000000000000000", 33 },
		{ "000000003ff00000000000000000000000000000", 34 },
		{ "000000001ff00000000000000000000000000000", 35 },
	};

	for (auto const& t : tests)
	{
		std::printf("%s\n", t.first);
		std::uint32_t buf[5];
		to_binary(t.first, buf);
		TEST_EQUAL(aux::count_leading_zeros_sw({buf, 5}), t.second);
#if TORRENT_HAS_BUILTIN_CLZ || defined _MSC_VER
		TEST_EQUAL(aux::count_leading_zeros_hw({buf, 5}), t.second);
#endif
		TEST_EQUAL(aux::count_leading_zeros({buf, 5}), t.second);
	}
}

TORRENT_TEST(count_trailing_ones_u32)
{
	std::uint32_t v = 0;
	TEST_EQUAL(aux::count_trailing_ones_sw(v), 0);
#if TORRENT_HAS_BUILTIN_CTZ || defined _MSC_VER
	TEST_EQUAL(aux::count_trailing_ones_hw(v), 0);
#endif
	TEST_EQUAL(aux::count_trailing_ones(v), 0);

	v = 0xffffffff;
	TEST_EQUAL(aux::count_trailing_ones_sw(v), 32);
#if TORRENT_HAS_BUILTIN_CTZ || defined _MSC_VER
	TEST_EQUAL(aux::count_trailing_ones_hw(v), 32);
#endif
	TEST_EQUAL(aux::count_trailing_ones(v), 32);

	v = aux::host_to_network(0xff00ff00);
	TEST_EQUAL(aux::count_trailing_ones_sw(v), 0);
#if TORRENT_HAS_BUILTIN_CTZ || defined _MSC_VER
	TEST_EQUAL(aux::count_trailing_ones_hw(v), 0);
#endif
	TEST_EQUAL(aux::count_trailing_ones(v), 0);

	v = aux::host_to_network(0xff0fff00);
	TEST_EQUAL(aux::count_trailing_ones_sw(v), 0);
#if TORRENT_HAS_BUILTIN_CTZ || defined _MSC_VER
	TEST_EQUAL(aux::count_trailing_ones_hw(v), 0);
#endif
	TEST_EQUAL(aux::count_trailing_ones(v), 0);

	v = aux::host_to_network(0xf0ff00ff);
	TEST_EQUAL(aux::count_trailing_ones_sw(v), 8);
#if TORRENT_HAS_BUILTIN_CTZ || defined _MSC_VER
	TEST_EQUAL(aux::count_trailing_ones_hw(v), 8);
#endif
	TEST_EQUAL(aux::count_trailing_ones(v), 8);

	v = aux::host_to_network(0xf0ff0fff);
	TEST_EQUAL(aux::count_trailing_ones_sw(v), 12);
#if TORRENT_HAS_BUILTIN_CTZ || defined _MSC_VER
	TEST_EQUAL(aux::count_trailing_ones_hw(v), 12);
#endif
	TEST_EQUAL(aux::count_trailing_ones(v), 12);

	std::uint32_t const arr[2] = {
		aux::host_to_network(0xf0ff0fff)
		, 0xffffffff};
	TEST_EQUAL(aux::count_trailing_ones_sw(arr), 44);
#if TORRENT_HAS_BUILTIN_CTZ || defined _MSC_VER
	TEST_EQUAL(aux::count_trailing_ones_hw(arr), 44);
#endif
	TEST_EQUAL(aux::count_trailing_ones(arr), 44);
}
