/*

Copyright (c) 2004, 2006, 2010, 2015, 2017-2021, Arvid Norberg
Copyright (c) 2016, Jan Berkel
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_VERSION_HPP_INCLUDED
#define TORRENT_VERSION_HPP_INCLUDED

#include "ip2/aux_/export.hpp"
#include <cstdint>

#define IP2_VERSION_MAJOR 0
#define IP2_VERSION_MINOR 0
#define IP2_VERSION_TINY 0

// the format of this version is: MMmmtt
// M = Major version, m = minor version, t = tiny version
#define IP2_VERSION_NUM ((IP2_VERSION_MAJOR * 10000) + (IP2_VERSION_MINOR * 100) + IP2_VERSION_TINY)

#define IP2_VERSION "0.0.0"

namespace ip2 {

	// the major, minor and tiny versions of ip2
	constexpr int version_major = 0;
	constexpr int version_minor = 0;
	constexpr int version_tiny = 0;

	// the ip2 version in string form
	constexpr char const* version_str = "0.0.0";

	// returns the ip2 version as string form in this format:
	// "<major>.<minor>.<tiny>.<tag>"
	TORRENT_EXPORT char const* version();

}

namespace lt = ip2;

#endif
