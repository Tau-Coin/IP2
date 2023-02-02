/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/context.hpp"

namespace ip2 {
namespace assemble {

namespace {
	static std::uint32_t s_context_id = 0;
}

context::context() : m_id(s_context_id++), m_error(api::NO_ERROR) {}

} // namespace assemble
} // namespace ip2
