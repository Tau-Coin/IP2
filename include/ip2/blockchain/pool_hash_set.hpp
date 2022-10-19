/*
Copyright (c) 2021, TaiXiang Cui
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_POOL_HASH_SET_HPP
#define IP2_POOL_HASH_SET_HPP


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
        class pool_hash_set {
        public:
            // @param Construct with entry
            explicit pool_hash_set(const entry &e);

            // @param Construct with bencode
            explicit pool_hash_set(std::string encode) : pool_hash_set(bdecode(encode)) {}

            explicit pool_hash_set(std::set<sha1_hash> mPoolHashSet) : m_pool_hash_set(std::move(mPoolHashSet)) {
                auto encode = get_encode();
                m_hash = hasher(encode).final();
            }

            const std::set<sha1_hash> &PoolHashSet() const { return m_pool_hash_set; }

            // @returns the SHA1 hash of this block
            const sha1_hash &sha1() const { return m_hash; }

            bool empty() const { return m_pool_hash_set.empty(); }

            entry get_entry() const;

            std::string get_encode() const;

            // @returns a pretty-printed string representation of block structure
            std::string to_string() const;

            friend std::ostream &operator<<(std::ostream &os, const pool_hash_set &hashArray);

        private:
            // populate hash array from entry
            void populate(const entry &e);

            // pool hash set
            std::set<sha1_hash> m_pool_hash_set;

            // sha1 hash
            sha1_hash m_hash;
        };
    }
}


#endif //IP2_POOL_HASH_SET_HPP
