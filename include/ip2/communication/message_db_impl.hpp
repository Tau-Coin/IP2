/*
Copyright (c) 2021, TaiXiang Cui
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef IP2_MESSAGE_DB_IMPL_HPP
#define IP2_MESSAGE_DB_IMPL_HPP


#include <sqlite3.h>
//#include <leveldb/db.h>

#include "ip2/communication/message_db_interface.hpp"

namespace ip2 {
    namespace communication {

        struct message_db_impl final : message_db_interface {

            explicit message_db_impl(sqlite3 *mSqlite) : m_sqlite(mSqlite) {}

            // init db
            bool init() override;

            bool create_table_friends() override;

            // get all friends
            std::vector<dht::public_key> get_all_friends() override;

            // save a friend in db
            bool save_friend(const dht::public_key &pubKey) override;

            // delete a friend
            bool delete_friend(const dht::public_key &pubKey) override;

            bool create_table_messages() override;

            bool save_message_if_not_exist(const message &msg) override;

            message get_message_by_hash(const sha1_hash &hash) override;

            communication::message
            get_latest_transaction(const dht::public_key &sender, const dht::public_key &receiver) override;

            std::vector<communication::message>
            get_latest_ten_transactions(const dht::public_key &sender, const dht::public_key &receiver) override;

            bool delete_message_by_hash(const sha1_hash &hash) override;

            bool is_message_in_db(const sha1_hash &hash) override;

        private:

            // sqlite3 instance
            sqlite3 *m_sqlite;

            // level db instance
//            leveldb::DB* m_leveldb;
        };
    }
}


#endif //IP2_MESSAGE_DB_IMPL_HPP
