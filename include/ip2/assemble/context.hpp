/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_CONTEXT_HPP
#define IP2_ASSEMBLE_CONTEXT_HPP

#include "ip2/config.hpp"
#include "ip2/aux_/common.h"
#include "ip2/api/error_code.hpp"

using namespace ip2::api;

namespace ip2 {
namespace assemble {

struct TORRENT_EXTRA_EXPORT context
{
public:

	context();

	std::uint32_t id() { return m_id; }

	void set_error(api::error_code e) { m_error = e; }

	api::error_code get_error() { return m_error; }

	virtual void done();

protected:

	std::uint32_t m_id;

	api::error_code m_error;
};

} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_CONTEXT_HPP
