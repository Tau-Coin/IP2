/*
Copyright (c) 2021, TaiXiang Cui
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_IMMUTABLE_INFO_HPP
#define IP2_IMMUTABLE_INFO_HPP

#include <utility>
#include <ostream>

#include <ip2/kademlia/node_entry.hpp>
#include <ip2/entry.hpp>
#include <ip2/aux_/common.h>
#include "ip2/aux_/common_data.h"
#include "ip2/sha1_hash.hpp"
#include "ip2/entry.hpp"

namespace ip2::communication {

    class immutable_data_info {
    public:
        immutable_data_info() = default;

        // @param Construct with entry
        explicit immutable_data_info(const entry& e);

        // construct online signal
        immutable_data_info(sha256_hash target, std::vector<dht::node_entry> entries) :
            m_target(std::move(target)), m_entries(std::move(entries)) {}

        // @returns target(immutable data hash)
        const sha256_hash &target() const { return m_target; }

        bool empty() const { return m_target.is_all_zeros(); }

        // @returns entries
        const std::vector<dht::node_entry> &entries() const { return m_entries; }

        // @returns the corresponding entry
        entry get_entry() const;

        // @returns a pretty-printed string representation of message structure
        std::string to_string() const;

        bool operator<(const immutable_data_info &rhs) const {
            return m_target < rhs.m_target;
        }

        bool operator>(const immutable_data_info &rhs) const {
            return rhs < *this;
        }

        bool operator<=(const immutable_data_info &rhs) const {
            return !(rhs < *this);
        }

        bool operator>=(const immutable_data_info &rhs) const {
            return !(*this < rhs);
        }

        friend std::ostream &operator<<(std::ostream &os, const immutable_data_info &info);

    private:
        // populate immutable data info from entry
        void populate(const entry& e);

        sha256_hash m_target;

        std::vector<dht::node_entry> m_entries;
    };
}


#endif //IP2_IMMUTABLE_INFO_HPP
