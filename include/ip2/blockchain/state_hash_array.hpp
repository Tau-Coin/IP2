/*
Copyright (c) 2021, TaiXiang Cui
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_STATE_HASH_ARRAY_HPP
#define IP2_STATE_HASH_ARRAY_HPP

#include <utility>

#include "ip2/aux_/common.h"
#include "ip2/aux_/common_data.h"
#include "ip2/hasher.hpp"
#include "ip2/entry.hpp"
#include "ip2/bencode.hpp"
#include "ip2/bdecode.hpp"
#include "ip2/sha1_hash.hpp"

namespace ip2 {
    namespace blockchain {
        class state_hash_array {
        public:
            state_hash_array() = default;

            // @param Construct with entry
            explicit state_hash_array(const entry& e);

            // @param Construct with bencode
            explicit state_hash_array(std::string encode): state_hash_array(bdecode(encode)) {}

            explicit state_hash_array(std::vector<sha1_hash> mHashArray) : m_hash_array(std::move(mHashArray)) {
                auto encode = get_encode();
                m_hash = hasher(encode).final();
            }

            const std::vector<sha1_hash> &HashArray() const { return m_hash_array; }

            // @returns the SHA1 hash of this block
            const sha1_hash &sha1() const { return m_hash; }

            bool empty() const { return m_hash_array.empty(); }

            entry get_entry() const;

            std::string get_encode() const;

            // @returns a pretty-printed string representation of block structure
            std::string to_string() const;

            friend std::ostream &operator<<(std::ostream &os, const state_hash_array &hashArray);

        private:
            // populate hash array from entry
            void populate(const entry& e);

            // hash
            std::vector<sha1_hash> m_hash_array;

            // sha1 hash
            sha1_hash m_hash;
        };
    }
}


#endif //IP2_STATE_HASH_ARRAY_HPP
