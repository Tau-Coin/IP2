/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_TRANSPORT_LOGGER_HPP
#define IP2_TRANSPORT_LOGGER_HPP

#include "ip2/config.hpp"
#include "ip2/aux_/common.h"

namespace ip2 {
namespace transport {

struct TORRENT_EXTRA_EXPORT transport_logger
{
	virtual bool should_log(aux::LOG_LEVEL log_level) const = 0;
	virtual void log(aux::LOG_LEVEL log_level, char const* fmt, ...) TORRENT_FORMAT(3,4) = 0;

protected:
	~transport_logger() = default;
};

} // namespace transport
} // namespace ip2

#endif // IP2_TRANSPORT_LOGGER_HPP
