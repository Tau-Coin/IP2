/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_URI_HPP
#define IP2_URI_HPP

#include "ip2/config.hpp"

#include <cstdint>
#include <algorithm>
#include <array>

namespace ip2 {
namespace aux {

struct uri
{
	uri() = default;

	explicit uri(char const* b)
	{ std::copy(b, b + len, bytes.begin()); }

	bool operator==(uri const& rhs) const
	{ return bytes == rhs.bytes; }

	bool operator!=(const uri &rhs) const
	{ return !(rhs == *this);}

	bool operator<(const uri &rhs) const
	{ return bytes < rhs.bytes; }

	bool operator>(const uri &rhs) const
	{	return rhs < *this; }

	bool operator<=(const uri &rhs) const
	{	return !(rhs < *this); }

	bool operator>=(const uri &rhs) const
	{	return !(*this < rhs); }

	bool is_all_zeros() const
	{
		return std::all_of(bytes.begin(), bytes.end()
			, [](char v) { return v == 0; }); 
	}

	static constexpr int len = 20; 
	std::array<char, len> bytes{};

};  

} // namespace aux
} // namespace ip2

#endif // IP2_URI_HPP
