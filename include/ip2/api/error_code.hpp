/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_API_ERROR_CODE_HPP
#define IP2_API_ERROR_CODE_HPP

namespace ip2 {
namespace api {

enum error_code
{
	NO_ERROR = 0,
	TRANSPORT_BUFFER_FULL,
	TRANSPORT_STOPPED,
	NETWORK_ERROR,
};

} // namespace api
} // namespace ip2

#endif // IP2_API_ERROR_CODE_HPP
