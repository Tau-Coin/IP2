/*

Copyright (c) 2005, 2007, 2009, 2013-2020, Arvid Norberg
Copyright (c) 2016-2018, Alden Torres
Copyright (c) 2017, Tim Niederhausen
Copyright (c) 2019, Fabrice Fontaine
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_BUFFER_HPP_INCLUDED
#define TORRENT_BUFFER_HPP_INCLUDED

#include <cstring>
#include <limits> // for numeric_limits
#include <cstdlib> // malloc/free/realloc
#include <algorithm> // for std::swap
#include <cstdint>

#include "ip2/aux_/invariant_check.hpp"
#include "ip2/assert.hpp"
#include "ip2/span.hpp"
#include "ip2/aux_/throw.hpp"

#if defined __GLIBC__
#include <malloc.h>
#elif defined _MSC_VER
#include <malloc.h>
#elif defined __FreeBSD__
#include <malloc_np.h>
#elif defined TORRENT_BSD
#include <malloc/malloc.h>
#endif

namespace ip2 {
namespace aux {

// the buffer is allocated once and cannot be resized. The size() may be
// larger than requested, in case the underlying allocator over allocated. In
// order to "grow" an allocation, create a new buffer and initialize it by
// the range of bytes from the existing, and move-assign the new over the
// old.
class buffer
{
public:
	using difference_type = std::ptrdiff_t;
	using index_type = std::ptrdiff_t;

	// allocate an uninitialized buffer of the specified size
	explicit buffer(difference_type size = 0)
	{
		TORRENT_ASSERT(size < (std::numeric_limits<std::int32_t>::max)());

		if (size == 0) return;

		// this rounds up the size to be 8 bytes aligned
		// it mostly makes sense for platforms without support
		// for a variation of "malloc_size()"
		size = (size + 7) & (~difference_type(0x7));

		// we have to use malloc here, to be compatible with the fancy query
		// functions below
		m_begin = static_cast<char*>(std::malloc(static_cast<std::size_t>(size)));
		if (m_begin == nullptr) aux::throw_ex<std::bad_alloc>();

		// the actual allocation may be larger than we requested. If so, let the
		// user take advantage of every single byte
#if (defined __GLIBC__ && !defined __UCLIBC__) || defined __FreeBSD__
		m_size = static_cast<difference_type>(::malloc_usable_size(m_begin));
#elif defined _MSC_VER
		m_size = static_cast<difference_type>(::_msize(m_begin));
#elif defined TORRENT_BSD
		m_size = static_cast<difference_type>(::malloc_size(m_begin));
#else
		m_size = size;
#endif
	}

	// allocate an uninitialized buffer of the specified size
	// and copy the initialization range into the start of the buffer
	buffer(difference_type const size, span<char const> initialize)
		: buffer(size)
	{
		TORRENT_ASSERT(initialize.size() <= size);
		if (!initialize.empty())
		{
			std::copy(initialize.begin(), initialize.begin()
				+ (std::min)(initialize.size(), size), m_begin);
		}
	}

	buffer(buffer const& b) = delete;

	buffer(buffer&& b)
		: m_begin(b.m_begin)
		, m_size(b.m_size)
	{
		b.m_begin = nullptr;
		b.m_size = 0;
	}

	buffer& operator=(buffer&& b)
	{
		if (&b == this) return *this;
		std::free(m_begin);
		m_begin = b.m_begin;
		m_size = b.m_size;
		b.m_begin = nullptr;
		b.m_size = 0;
		return *this;
	}

	buffer& operator=(buffer const& b) = delete;

	~buffer() { std::free(m_begin); }

	char* data() { return m_begin; }
	char const* data() const { return m_begin; }
	difference_type size() const { return m_size; }

	bool empty() const { return m_size == 0; }
	char& operator[](index_type const i) { TORRENT_ASSERT(i < size()); return m_begin[i]; }
	char const& operator[](difference_type const i) const { TORRENT_ASSERT(i < size()); return m_begin[i]; }

	char* begin() { return m_begin; }
	char const* begin() const { return m_begin; }
	char* end() { return m_begin + m_size; }
	char const* end() const { return m_begin + m_size; }

	void swap(buffer& b)
	{
		using std::swap;
		swap(m_begin, b.m_begin);
		swap(m_size, b.m_size);
	}

private:
	char* m_begin = nullptr;
	// m_begin points to an allocation of this size.
	difference_type m_size = 0;
};

}
}

#endif // TORRENT_BUFFER_HPP_INCLUDED
