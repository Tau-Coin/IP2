/*

Copyright (c) 2007, Un Shyam
Copyright (c) 2007-2009, 2011-2012, 2014-2020, Arvid Norberg
Copyright (c) 2016-2017, 2021, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_PE_CRYPTO_HPP_INCLUDED
#define TORRENT_PE_CRYPTO_HPP_INCLUDED

#if !defined TORRENT_DISABLE_ENCRYPTION

#include "ip2/config.hpp"

#include "ip2/aux_/disable_warnings_push.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include "ip2/aux_/disable_warnings_pop.hpp"

#include "ip2/aux_/receive_buffer.hpp"
#include "ip2/sha1_hash.hpp"
#include "ip2/extensions.hpp"
#include "ip2/assert.hpp"
#include "ip2/span.hpp"
#include "ip2/aux_/array.hpp"

#include <list>
#include <array>
#include <cstdint>

namespace ip2::aux {

	namespace mp = boost::multiprecision;

	using key_t = mp::number<mp::cpp_int_backend<768, 768, mp::unsigned_magnitude, mp::unchecked, void>>;

	TORRENT_EXTRA_EXPORT std::array<char, 96> export_key(key_t const& k);

	// RC4 state from libtomcrypt
	struct rc4 {
		int x;
		int y;
		aux::array<std::uint8_t, 256> buf;
	};

	// TODO: 3 dh_key_exchange should probably move into its own file
	class TORRENT_EXTRA_EXPORT dh_key_exchange
	{
	public:
		dh_key_exchange();
		bool good() const { return true; }

		// Get local public key
		key_t const& get_local_key() const { return m_dh_local_key; }

		// read remote_pubkey, generate and store shared secret in
		// m_dh_shared_secret.
		void compute_secret(std::uint8_t const* remote_pubkey);
		void compute_secret(key_t const& remote_pubkey);

		key_t const& get_secret() const { return m_dh_shared_secret; }

		sha1_hash const& get_hash_xor_mask() const { return m_xor_mask; }

	private:

		key_t m_dh_local_key;
		key_t m_dh_local_secret;
		key_t m_dh_shared_secret;
		sha1_hash m_xor_mask;
	};

	struct TORRENT_EXTRA_EXPORT encryption_handler
	{
		std::tuple<int, span<span<char const>>>
		encrypt(span<span<char>> iovec);

		int decrypt(aux::crypto_receive_buffer& recv_buffer
			, std::size_t& bytes_transferred);

		bool switch_send_crypto(std::shared_ptr<crypto_plugin> crypto
			, int pending_encryption);

		void switch_recv_crypto(std::shared_ptr<crypto_plugin> crypto
			, aux::crypto_receive_buffer& recv_buffer);

		bool is_send_plaintext() const
		{
			return m_send_barriers.empty() || m_send_barriers.back().next != INT_MAX;
		}

		bool is_recv_plaintext() const
		{
			return m_dec_handler.get() == nullptr;
		}

	private:
		struct barrier
		{
			barrier(std::shared_ptr<crypto_plugin> plugin, int n)
				: enc_handler(plugin), next(n) {}
			std::shared_ptr<crypto_plugin> enc_handler;
			// number of bytes to next barrier
			int next;
		};
		std::list<barrier> m_send_barriers;
		std::shared_ptr<crypto_plugin> m_dec_handler;
	};

	struct TORRENT_EXTRA_EXPORT rc4_handler : crypto_plugin
	{
	public:
		rc4_handler();

		// Input keys must be 20 bytes
		void set_incoming_key(span<char const> key) override;
		void set_outgoing_key(span<char const> key) override;

		std::tuple<int, span<span<char const>>>
		encrypt(span<span<char>> buf) override;

		std::tuple<int, int, int> decrypt(span<span<char>> buf) override;

	private:
		rc4 m_rc4_incoming;
		rc4 m_rc4_outgoing;

		// determines whether or not encryption and decryption is enabled
		bool m_encrypt;
		bool m_decrypt;
	};

} // namespace ip2::aux

#endif // TORRENT_DISABLE_ENCRYPTION

#endif // TORRENT_PE_CRYPTO_HPP_INCLUDED
