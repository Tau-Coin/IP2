/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/rpc_params_config.hpp"

namespace ip2 {
namespace assemble {

namespace {

	static const std::map<api::dht_rpc_type, api::dht_rpc_params> s_rpc_params_config =
	{
		{api::PUT, api::dht_rpc_params{1, 8, 16, 0} },
		{api::GET, api::dht_rpc_params{1, 8, 16, 0} },
		{api::RELAY, api::dht_rpc_params{1, 8, 16, 3} },
	};

}

	api::dht_rpc_params get_rpc_parmas(api::dht_rpc_type t)
	{
		auto it = s_rpc_params_config.find(t);
		if (it != s_rpc_params_config.end()) return it->second;

		return api::dht_rpc_params{0, 0, 0, 0};
	}

} // namespace assemble
} // namespace ip2
