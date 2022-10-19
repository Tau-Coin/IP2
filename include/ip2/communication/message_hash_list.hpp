/*
Copyright (c) 2021, TaiXiang Cui
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_MESSAGE_HASH_LIST_HPP
#define IP2_MESSAGE_HASH_LIST_HPP


#include <ostream>
#include "ip2/aux_/common.h"
#include "ip2/aux_/common_data.h"
#include "ip2/aux_/export.hpp"
#include "ip2/entry.hpp"
#include "ip2/bencode.hpp"
#include "ip2/bdecode.hpp"
#include "ip2/sha1_hash.hpp"
#include "ip2/hasher.hpp"

namespace ip2::communication {

    // The ``message_hash_list`` class is a list, encode/decode message hash list
    class TORRENT_EXPORT message_hash_list {
    public:

        // @param Construct with entry
        explicit message_hash_list(const entry& e);

        // @param Construct with bencode
        explicit message_hash_list(std::string encode): message_hash_list(bdecode(encode)) {}

        explicit message_hash_list(std::vector<sha1_hash> message_hash_list);

        // @returns all message hash in this container
        std::vector<sha1_hash> hash_list() const { return m_message_hash_list; }

        entry get_entry() const;

        // @returns the SHA1 hash
        sha1_hash sha1();

        // @returns the bencode
        std::string encode();

        // @returns a pretty-printed string representation of message structure
        std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const message_hash_list &list);

    private:
        // populate hash list data from entry
        void populate(const entry& e);

        // message hash list
        std::vector<sha1_hash> m_message_hash_list;
    };
}


#endif //IP2_MESSAGE_HASH_LIST_HPP
