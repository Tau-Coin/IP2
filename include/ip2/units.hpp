/*

Copyright (c) 2016-2021, Arvid Norberg
Copyright (c) 2016-2017, Alden Torres
Copyright (c) 2019, Steven Siloti
Copyright (c) 2020, Silver Zachara
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_UNITS_HPP
#define TORRENT_UNITS_HPP

#include <cstdint>
#include <string>
#include <limits>
#include <iosfwd>
#include <type_traits>

#include "ip2/config.hpp"

namespace ip2 {
namespace aux {
	template <typename Tag>
	struct difference_tag;

#if TORRENT_USE_IOSTREAM
	template <typename T>
	struct type_to_print_as
	{
		using type = typename std::conditional<sizeof(T) < sizeof(int), int, T>::type;
	};
#endif


	template<typename UnderlyingType, typename Tag
		, typename Cond = typename std::enable_if<std::is_integral<UnderlyingType>::value>::type>
	struct strong_typedef
	{
		using underlying_type = UnderlyingType;
		using diff_type = strong_typedef<UnderlyingType, difference_tag<Tag>>;

		constexpr strong_typedef(strong_typedef const& rhs) noexcept = default;
		constexpr strong_typedef(strong_typedef&& rhs) noexcept = default;
		strong_typedef() noexcept = default;
#if TORRENT_ABI_VERSION == 1
		constexpr strong_typedef(UnderlyingType val) : m_val(val) {}
		constexpr operator UnderlyingType() const { return m_val; }
#else
		constexpr explicit strong_typedef(UnderlyingType val) : m_val(val) {}
		constexpr explicit operator UnderlyingType() const { return m_val; }
		constexpr bool operator==(strong_typedef const& rhs) const { return m_val == rhs.m_val; }
		constexpr bool operator!=(strong_typedef const& rhs) const { return m_val != rhs.m_val; }
		constexpr bool operator<(strong_typedef const& rhs) const { return m_val < rhs.m_val; }
		constexpr bool operator>(strong_typedef const& rhs) const { return m_val > rhs.m_val; }
		constexpr bool operator>=(strong_typedef const& rhs) const { return m_val >= rhs.m_val; }
		constexpr bool operator<=(strong_typedef const& rhs) const { return m_val <= rhs.m_val; }
#endif
		strong_typedef& operator++() { ++m_val; return *this; }
		strong_typedef& operator--() { --m_val; return *this; }

		strong_typedef operator++(int) & { return strong_typedef{m_val++}; }
		strong_typedef operator--(int) & { return strong_typedef{m_val--}; }

		friend diff_type operator-(strong_typedef lhs, strong_typedef rhs)
		{ return diff_type{lhs.m_val - rhs.m_val}; }
		friend strong_typedef operator+(strong_typedef lhs, diff_type rhs)
		{ return strong_typedef{lhs.m_val + static_cast<UnderlyingType>(rhs)}; }
		friend strong_typedef operator+(diff_type lhs, strong_typedef rhs)
		{ return strong_typedef{static_cast<UnderlyingType>(lhs) + rhs.m_val}; }
		friend strong_typedef operator-(strong_typedef lhs, diff_type rhs)
		{ return strong_typedef{lhs.m_val - static_cast<UnderlyingType>(rhs)}; }

		strong_typedef& operator+=(diff_type rhs) &
		{ m_val += static_cast<UnderlyingType>(rhs); return *this; }
		strong_typedef& operator-=(diff_type rhs) &
		{ m_val -= static_cast<UnderlyingType>(rhs); return *this; }

		strong_typedef& operator=(strong_typedef const& rhs) & noexcept = default;
		strong_typedef& operator=(strong_typedef&& rhs) & noexcept = default;

#if TORRENT_USE_IOSTREAM
		friend std::ostream& operator<<(std::ostream& os, strong_typedef val)
		{ return os << static_cast<typename type_to_print_as<UnderlyingType>::type>(static_cast<UnderlyingType>(val)); }
#endif

	private:
		UnderlyingType m_val;
	};

	// meta function to return the underlying type of a strong_typedef or enumeration
	// , or the type itself if it isn't a strong_typedef
	template <typename T, typename = void>
	struct underlying_index_t { using type = T; };

	template <typename T>
	struct underlying_index_t<T, typename std::enable_if<std::is_enum<T>::value>::type>
	{ using type = typename std::underlying_type<T>::type; };

	template <typename U, typename Tag>
	struct underlying_index_t<aux::strong_typedef<U, Tag>> { using type = U; };

	struct piece_index_tag;
	struct file_index_tag;

	template <typename T, typename Tag>
	std::string to_string(strong_typedef<T, Tag> const t)
	{ return std::to_string(static_cast<T>(t)); }

	template <typename T, typename Tag>
	strong_typedef<T, Tag> next(strong_typedef<T, Tag> v)
	{ return ++v;}

	template <typename T, typename Tag>
	strong_typedef<T, Tag> prev(strong_typedef<T, Tag> v)
	{ return --v;}

} // namespace ip2::aux

	// this type represents a piece index in a torrent.
	using piece_index_t = aux::strong_typedef<std::int32_t, aux::piece_index_tag>;

	// this type represents an index to a file in a torrent. Any specific torrent
	// file has a well defined and immutable file list, and a file index into it
	// always refers to the same file.
	using file_index_t = aux::strong_typedef<std::int32_t, aux::file_index_tag>;

} // namespace ip2

namespace std {

	template<typename UnderlyingType, typename Tag>
	class numeric_limits<ip2::aux::strong_typedef<UnderlyingType, Tag>> : public std::numeric_limits<UnderlyingType>
	{
		using type = ip2::aux::strong_typedef<UnderlyingType, Tag>;
	public:

		static constexpr type (min)()
		{ return type((std::numeric_limits<UnderlyingType>::min)()); }

		static constexpr type (max)()
		{ return type((std::numeric_limits<UnderlyingType>::max)()); }
	};

	template<typename UnderlyingType, typename Tag>
	struct hash<ip2::aux::strong_typedef<UnderlyingType, Tag>> : std::hash<UnderlyingType>
	{
		using base = std::hash<UnderlyingType>;
		using result_type = std::size_t;
		result_type operator()(ip2::aux::strong_typedef<UnderlyingType, Tag> const& s) const
		{ return this->base::operator()(static_cast<UnderlyingType>(s)); }
	};
}

#endif
