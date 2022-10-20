/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_TRANSPORT_DHT_RPC_HPP
#define IP2_TRANSPORT_DHT_RPC_HPP

#include "ip2/entry.hpp"

#include <ip2/kademlia/node_id.hpp>
#include <ip2/kademlia/types.hpp>

#include <string>

namespace ip2 {
namespace transport {

struct rpc_ctx
{
	explicit rpc_ctx(std::int8_t invoke_branch, std::int8_t invoke_window
		, std::int8_t invoke_limit)
		: m_invoke_branch(invoke_branch)
		, m_invoke_window(invoke_window)
		, m_invoke_limit(invoke_limit)
	{}

	std::int8_t m_invoke_branch;
	std::int8_t m_invoke_window;
	std::int8_t m_invoke_limit;
};

struct get_ctx : rpc_ctx
{
	explicit get_ctx(dht::public_key const& pubkey, std::string const& salt
		, std::int64_t timestamp, std::int8_t invoke_branch, std::int8_t invoke_window
		, std::int8_t invoke_limit)
		: rpc_ctx(invoke_branch, invoke_window, invoke_limit)
		, m_pubkey(pubkey)
		, m_salt(salt)
		, m_timestamp(timestamp)
	{}

	public_key m_pubkey;
	std::string m_salt;
	std::int64_t m_timestamp;
};

struct put_ctx : rpc_ctx
{
	explicit put_ctx(entry data, std::string const& salt
		, std::int8_t invoke_branch, std::int8_t invoke_window
		, std::int8_t invoke_limit)
		: rpc_ctx(invoke_branch, invoke_window, invoke_limit)
		, m_data(std::move(data))
		, m_salt(salt)
	{}

	entry m_data;
	std::string m_salt;
};

struct relay_ctx : rpc_ctx
{
	explicit relay_ctx(public_key const& to, entry payload
		, std::int8_t invoke_branch, std::int8_t invoke_window
		, std::int8_t invoke_limit)
		: rpc_ctx(invoke_branch, invoke_window, invoke_limit)
		, m_to(to)
		, m_payload(std::move(payload))
	{}

	public_key m_to;
	entry m_payload;
};

using rpc_method = std::function<void(void)>;

struct rpc
{
	explicit rpc(rpc_method method) : m_method(std::move(method)) {}

	rpc_method m_method;
};

} // namespace transport
} // namespace ip2

#endif // IP2_TRANSPORT_DHT_RPC_HPP
