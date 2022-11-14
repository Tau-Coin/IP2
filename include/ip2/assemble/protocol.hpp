/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_ASSEMBLE_PROTOCOL_HPP
#define IP2_ASSEMBLE_PROTOCOL_HPP

#include "ip2/config.hpp"
#include "ip2/aux_/common.h"
#include "ip2/api/error_code.hpp"
#include "ip2/entry.hpp"
#include "ip2/uri.hpp"
#include "ip2/sha1_hash.hpp"

#include "ip2/kademlia/types.hpp"

#include <string>
#include <tuple>

using namespace ip2::api;

namespace ip2 {
namespace assemble {
namespace protocol {

/*
		blob seg protcol:
		{
			'v': <version number with 4 bytes>
			'n': 's' // segment
			'a': {
				'v': <segment value>
				'n': <next segment hash>
			}
		}

		relay uri protcol:
		{
			'v': <version number with 4 bytes>
			'n': 'u' // uri
			'a': {
				's': <uri sender public key>
				'u': <uri>
				'ts': <timestamp>
			}
		}

		relay message protcol:
		{
			'v': <version number with 4 bytes>
			'n': 'm' // message
			'a': {
				'm': <message content>
			}
		}
 */

	constexpr int version_length = 4;

	inline bool version_match(std::string const& ver, std::string const& pro_ver)
	{
		return strncmp(pro_ver.c_str(), ver.c_str(), 2) == 0 ? true : false;
	}

	static const std::int32_t blob_mtu = 10 * 1024;
	static const std::int32_t blob_seg_mtu = 950;
	static const std::int32_t relay_msg_mtu = 950;

	struct basic_protocol
	{
	public:

		static std::string version;
		static std::string name;

		basic_protocol();

		basic_protocol(std::string const& ver, std::string const& n);

		entry to_entry();

		std::string get_version() { return m_version; }
		std::string get_name() { return m_name; }

	protected:

		std::string m_version;
		std::string m_name;

		entry m_arg;

	private:

		static const int major = 0;
		static const int minor = 0;
		static const int tiny = 0;
		static char const ver[];
	};

	struct blob_seg_protocol : basic_protocol
	{
	public:

		static std::string version;
		static std::string name;

		blob_seg_protocol(std::string const& ver, std::string const& n
			, std::string const& seg_value, sha1_hash const& next_pointer);

		blob_seg_protocol(std::string const& seg_value, sha1_hash const& next_pointer);

		std::string seg_value() { return m_seg_value; }
		sha1_hash next_pointer() { return m_next_pointer; }

	protected:

		std::string m_seg_value;
		sha1_hash m_next_pointer;

	private:

 		static const int major = 0;
		static const int minor = 0;
		static const int tiny = 0;
		static char const ver[];
	};

	struct relay_uri_protocol : basic_protocol
	{
	public:

		static std::string version;
		static std::string name;

		relay_uri_protocol(std::string const& ver, std::string const& n
			, dht::public_key const& pk, aux::uri const& blob_uri
			, dht::timestamp ts);

		relay_uri_protocol(dht::public_key const& pk, aux::uri const& blob_uri
			, dht::timestamp ts);

		dht::public_key pk() { return m_pk; }
		aux::uri blob_uri() { return m_uri; }
		dht::timestamp ts() { return m_ts; }

	protected:

		dht::public_key m_pk;
		aux::uri m_uri;
		dht::timestamp m_ts;

	private:

		static const int major = 0;
		static const int minor = 0;
		static const int tiny = 0;
		static char const ver[];
	};

	struct relay_msg_protocol : basic_protocol
	{
	public:

		static std::string version;
		static std::string name;

		relay_msg_protocol(std::string const& ver, std::string const& n
			, std::string const& msg);

		relay_msg_protocol(std::string const& msg);

		std::string msg() { return m_msg; }

	protected:

		std::string m_msg;

	private:

		static const int major = 0;
		static const int minor = 0;
		static const int tiny = 0;
		static char const ver[];
	};

	// basic protocol factory method
	std::tuple<basic_protocol, api::error_code> construct_protocol(entry const& proto);

} // namespace protocol
} // namespace assemble
} // namespace ip2

#endif // IP2_ASSEMBLE_PROTOCOL_HPP
