#ifndef TORRENT_SHA512_HPP_INCLUDED
#define TORRENT_SHA512_HPP_INCLUDED

#include "ip2/config.hpp"

#if !defined TORRENT_USE_LIBGCRYPT \
	&& !TORRENT_USE_COMMONCRYPTO \
	&& !TORRENT_USE_CNG \
	&& !TORRENT_USE_CRYPTOAPI_SHA_512 \
	&& !defined TORRENT_USE_LIBCRYPTO

#include <cstdint>

namespace ip2 {
namespace aux {

	struct sha512_ctx
	{
		std::uint64_t length;
		std::uint64_t state[8];
		std::size_t curlen;
		std::uint8_t buf[128];
	};

	TORRENT_EXTRA_EXPORT int SHA512_init(sha512_ctx* md);
	TORRENT_EXTRA_EXPORT int SHA512_update(sha512_ctx* md
		, std::uint8_t const* in, std::size_t len);
	TORRENT_EXTRA_EXPORT int SHA512_final(std::uint8_t* out, sha512_ctx* md);
}
}

#endif
#endif
