/*

Copyright (c) 2021, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_CRYPTO_HPP_INCLUDED
#define TORRENT_CRYPTO_HPP_INCLUDED

#include "ip2/config.hpp"
#include "ip2/span.hpp"

#include <string>

namespace ip2 {

namespace aux {

	// AES encrypiton.
	// Here use std::string type compatible with OPENSSL AES suit.
	TORRENT_EXPORT bool aes_encrypt(const std::string& in
		, std::string& out
		, const std::string& key
		, std::string& err_str);

	// AES decrypiton.
	// Here use std::string type compatible with OPENSSL AES suit.
	TORRENT_EXPORT bool aes_decrypt(const std::string& in
		, std::string& out
		, const std::string& key
		, std::string& err_str);

	} // namespace aux
} // namespace ip2

#endif // TORRENT_CRYPTO_HPP_INCLUDED
