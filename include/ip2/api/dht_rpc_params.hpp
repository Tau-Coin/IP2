/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_API_DHT_RPC_PARAMETERS_HPP
#define IP2_API_DHT_RPC_PARAMETERS_HPP

#include "ip2/config.hpp"
#include "ip2/aux_/common.h"

namespace ip2 {
namespace api {

enum TORRENT_EXTRA_EXPORT dht_rpc_type
{
	PUT = 0,
	GET,
	RELAY,
};

struct TORRENT_EXTRA_EXPORT dht_rpc_params
{
	dht_rpc_params(std::int8_t b, std::int8_t w, std::int8_t l, std::int8_t hl)
		: invoke_branch(b), invoke_window(w), invoke_limit(l), hit_limit(hl)
	{}

	std::int8_t invoke_branch;
	std::int8_t invoke_window;
	std::int8_t invoke_limit;
	std::int8_t hit_limit;
};

} // namespace api
} // namespace ip2

#endif // IP2_API_DHT_RPC_PARAMETERS_HPP
