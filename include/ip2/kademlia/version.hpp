/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_DHT_VERSION_HPP
#define IP2_DHT_VERSION_HPP

#include <cstring>
#include <string>

namespace ip2 {
namespace dht {

    constexpr int major = 0;
    constexpr int minor = 0;
    constexpr int tiny = 0;

	constexpr int version_length = 4;

	static char const ver[] = { 'T', major, minor, tiny };
	static std::string version(ver, ver + version_length);

	inline bool version_match(const std::string& ver)
	{
		return strncmp(version.c_str(), ver.c_str(), 2) == 0 ? true : false;
	}

} // namespace dht
} // namespace ip2

#endif // IP2_DHT_VERSION_HPP
