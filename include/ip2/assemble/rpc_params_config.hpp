/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_RPC_PARAMS_CONFIG_HPP
#define IP2_ASSEMBLE_RPC_PARAMS_CONFIG_HPP

#include "ip2/config.hpp"
#include "ip2/aux_/common.h"

#include "ip2/api/dht_rpc_params.hpp"

#include <map>

using namespace ip2::api;

namespace ip2 {
namespace assemble {

api::dht_rpc_params get_rpc_parmas(api::dht_rpc_type t);

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_RPC_PARAMS_CONFIG_HPP
