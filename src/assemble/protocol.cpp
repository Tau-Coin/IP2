/*

Copyright (c) 2022, Xianshui Sheng
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "ip2/assemble/protocol.hpp"

#ifndef TORRENT_DISABLE_LOGGING
#include "ip2/hex.hpp" // to_hex
#endif

namespace ip2 {
namespace assemble {
namespace protocol {

char const basic_protocol::ver[] = { 'B'
	, basic_protocol::major, basic_protocol::minor, basic_protocol::tiny };

std::string basic_protocol::version = std::string(basic_protocol::ver
	, basic_protocol::ver + version_length);

std::string basic_protocol::name = "b";

basic_protocol::basic_protocol()
	: m_version(basic_protocol::version)
	, m_name(basic_protocol::name)
{}

basic_protocol::basic_protocol(std::string const& ver, std::string const& n)
	: m_version(ver), m_name(n)
{}

entry basic_protocol::to_entry()
{
	entry proto;

	proto["v"] = m_version;
	proto["n"] = m_name;
	proto["a"] = m_arg;

	return proto;
}

char const blob_seg_protocol::ver[] = { 'S'
	, blob_seg_protocol::major, blob_seg_protocol::minor, blob_seg_protocol::tiny };

std::string blob_seg_protocol::version = std::string(blob_seg_protocol::ver
	, blob_seg_protocol::ver + version_length);

std::string blob_seg_protocol::name = "s";

blob_seg_protocol::blob_seg_protocol(std::string const& ver, std::string const& n
	, std::string const& seg_value)
	: basic_protocol(ver, n)
	, m_seg_value(seg_value)
{
	m_arg["v"] = m_seg_value;
}

blob_seg_protocol::blob_seg_protocol(std::string const& seg_value)
	: basic_protocol(version, name)
	, m_seg_value(seg_value)
{
	m_arg["v"] = m_seg_value;
}

char const blob_index_protocol::ver[] = { 'I'
    , blob_index_protocol::major, blob_index_protocol::minor, blob_index_protocol::tiny };

std::string blob_index_protocol::version = std::string(blob_index_protocol::ver
    , blob_index_protocol::ver + version_length);

std::string blob_index_protocol::name = "i";

blob_index_protocol::blob_index_protocol(std::string const& ver, std::string const& n
	, std::vector<sha1_hash> const& hashes)
	: basic_protocol(ver, n)
{
	std::string hashes_str;

	for (auto& h : hashes)
	{
		m_seg_hashes.push_back(h);
		hashes_str.append(h.data(), 20);
	}

	m_arg["h"] = hashes_str;
}

blob_index_protocol::blob_index_protocol(std::vector<sha1_hash> const& hashes)
	: basic_protocol(version, name)
{
	std::string hashes_str;

	for (auto& h : hashes)
	{
		m_seg_hashes.push_back(h);
		hashes_str.append(h.data(), 20);
	}

	m_arg["h"] = hashes_str;
}

char const relay_uri_protocol::ver[] = { 'U'
	, relay_uri_protocol::major, relay_uri_protocol::minor, relay_uri_protocol::tiny };

std::string relay_uri_protocol::version = std::string(relay_uri_protocol::ver
	, relay_uri_protocol::ver + version_length);

std::string relay_uri_protocol::name = "u";

relay_uri_protocol::relay_uri_protocol(dht::public_key const& pk, aux::uri const& blob_uri
	, dht::timestamp ts)
	: basic_protocol(version, name)
	, m_pk(pk)
	, m_uri(blob_uri)
	, m_ts(ts)
{
	m_arg["s"] = m_pk.bytes;
	m_arg["u"] = m_uri.bytes;
	m_arg["ts"] = m_ts.value;
}

relay_uri_protocol::relay_uri_protocol(std::string const& ver, std::string const& n
	, dht::public_key const& pk, aux::uri const& blob_uri
	, dht::timestamp ts) 
	: basic_protocol(ver, n)
	, m_pk(pk)
	, m_uri(blob_uri)
	, m_ts(ts)
{
	m_arg["s"] = m_pk.bytes;
	m_arg["u"] = m_uri.bytes;
	m_arg["ts"] = m_ts.value;
}

char const relay_msg_protocol::ver[] = { 'M'
	, relay_msg_protocol::major, relay_msg_protocol::minor, relay_msg_protocol::tiny };

std::string relay_msg_protocol::version = std::string(relay_msg_protocol::ver
	, relay_msg_protocol::ver + version_length);

std::string relay_msg_protocol::name = "m";

relay_msg_protocol::relay_msg_protocol(std::string const& msg)
	: basic_protocol(version, name)
	, m_msg(msg)
{
	m_arg["m"] = m_msg;
}

relay_msg_protocol::relay_msg_protocol(std::string const& ver, std::string const& n
	, std::string const& msg)
	: basic_protocol(ver, n)
	, m_msg(msg)
{
	m_arg["m"] = m_msg;
}

// basic protocol factory method
std::tuple<std::shared_ptr<basic_protocol>, api::error_code>
		construct_protocol(entry const& proto, assemble_logger& logger)
{
	std::string version_str;
	std::string name_str;

	// get protocol version and name
	entry const* ver = proto.find_key("v");
	if (ver && ver->type() == entry::string_t
		&& ver->string().size() == version_length)
	{
		version_str.append(ver->string().data(), version_length);
	}
	else
	{
		return std::make_tuple(std::make_shared<basic_protocol>(), api::ASSEMBLE_VERSION_ERROR);
	}

	entry const* n = proto.find_key("n");
 	if (n && n->type() == entry::string_t
		&& n->string().size() == 1)
	{
		name_str.append(n->string().data(), 1);
	}
	else
	{
		return std::make_tuple(std::make_shared<basic_protocol>()
			, api::ASSEMBLE_PROTOCOL_FORMAT_ERROR);
	}

#ifndef TORRENT_DISABLE_LOGGING
	logger.log(aux::LOG_INFO, "parse protocol name:%s, version:%s"
		, name_str.c_str(), version_str.c_str());
#endif

	entry const* a = proto.find_key("a");
	if (!a || a->type() != entry::dictionary_t)
	{
		return std::make_tuple(std::make_shared<basic_protocol>()
			, api::ASSEMBLE_PROTOCOL_FORMAT_ERROR);
	}

	if (strncmp(name_str.c_str(), blob_seg_protocol::name.c_str(), 1) == 0)
	{
		if (!version_match(version_str, blob_seg_protocol::version))
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				 , api::ASSEMBLE_PROTOCOL_VER_MISMATCH);
		}

		std::string seg;

		entry const* se = a->find_key("v");
		if (se && se->type() == entry::string_t
			&& se->string().size() <= blob_seg_mtu)
		{
			seg.append(se->string().data(), se->string().size());
		}
		else
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				, api::ASSEMBLE_PROTOCOL_FORMAT_ERROR);
		}

		return std::make_tuple(std::make_shared<blob_seg_protocol>(version_str, name_str, seg)
			, api::NO_ERROR);
	}
	else if (strncmp(name_str.c_str(), blob_index_protocol::name.c_str(), 1) == 0)
	{
		if (!version_match(version_str, blob_index_protocol::version))
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				, api::ASSEMBLE_PROTOCOL_VER_MISMATCH);
		}

		std::string hash_str;
		std::vector<sha1_hash> hashes;

		entry const* he = a->find_key("h");
		if (he && he->type() == entry::string_t
			&& he->string().size() % 20 == 0)
		{
			hash_str.append(he->string().data(), he->string().size());

			int count = hash_str.size() / 20;
			for (int i = 0; i != count; i++)
			{
				sha1_hash h;
				std::memcpy(h.data(), hash_str.data() + i * 20, 20);
				hashes.push_back(h);
			}
		}
		else
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				, api::ASSEMBLE_PROTOCOL_FORMAT_ERROR);
		}

		return std::make_tuple(std::make_shared<blob_index_protocol>(version_str, name_str, hashes)
			, api::NO_ERROR);
	}
	else if (strncmp(name_str.c_str(), relay_uri_protocol::name.c_str(), 1) == 0)
	{
		if (!version_match(version_str, relay_uri_protocol::version))
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				, api::ASSEMBLE_PROTOCOL_VER_MISMATCH);
		}

		dht::public_key sender;
		aux::uri blob_uri;
		ip2::dht::timestamp ts;

		entry const* se = a->find_key("s");
		if (se && se->type() == entry::string_t
			&& se->string().size() == dht::public_key::len)
		{
			std::memcpy(sender.bytes.data(), se->string().data(), dht::public_key::len);
		}
		else
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				, api::ASSEMBLE_PROTOCOL_FORMAT_ERROR);
		}

		entry const* ue = a->find_key("u");
		if (ue && ue->type() == entry::string_t
			&& ue->string().size() == aux::uri::len)
		{
			std::memcpy(blob_uri.bytes.data(), ue->string().data(), aux::uri::len);
		}
		else
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				, api::ASSEMBLE_PROTOCOL_FORMAT_ERROR);
		}

		entry const* te = a->find_key("ts");
		if (te)
		{
			ts = ip2::dht::timestamp(te->integer());
		}

		return std::make_tuple(
			std::make_shared<relay_uri_protocol>(version_str, name_str, sender, blob_uri, ts)
			, api::NO_ERROR);
	}
	else if (strncmp(name_str.c_str(), relay_msg_protocol::name.c_str(), 1) == 0)
	{
		if (!version_match(version_str, relay_msg_protocol::version))
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				, api::ASSEMBLE_PROTOCOL_VER_MISMATCH);
		}

		std::string msg;

		entry const* me = a->find_key("m");
		if (me && me->type() == entry::string_t
			&& me->string().size() <= relay_msg_mtu)
		{
			msg.append(me->string().data(), me->string().size());
		}
		else
		{
			return std::make_tuple(std::make_shared<basic_protocol>()
				, api::ASSEMBLE_PROTOCOL_FORMAT_ERROR);
		}

		return std::make_tuple(std::make_shared<relay_msg_protocol>(version_str, name_str, msg)
			, api::NO_ERROR);
	}
	else
	{
		return std::make_tuple(std::make_shared<basic_protocol>()
			, api::ASSEMBLE_NAME_ERROR);
	}
}

} // namespace protocol
} // namespace assemble
} // namespace ip2
