/*

Copyright (c) 2019-2020, Arvid Norberg
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/gzip.hpp"

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
	lt::error_code ec;
	std::vector<char> out;
	lt::inflate_gzip({reinterpret_cast<char const*>(data), int(size)}, out
		, 100000, ec);
	return 0;
}

