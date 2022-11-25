/*

Copyright (c) 2017-2018, Steven Siloti
Copyright (c) 2017-2021, Arvid Norberg
Copyright (c) 2020, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.

*/

#ifndef TORRENT_FWD_HPP
#define TORRENT_FWD_HPP

#include "ip2/config.hpp"

namespace ip2 {

// include/ip2/alert.hpp
struct alert;

// include/ip2/alert_types.hpp
struct dht_routing_bucket;
struct udp_error_alert;
struct external_ip_alert;
struct listen_failed_alert;
struct listen_succeeded_alert;
struct portmap_error_alert;
struct portmap_alert;
struct portmap_log_alert;
struct dht_announce_alert;
struct dht_get_peers_alert;
struct dht_bootstrap_alert;
struct session_start_over_alert;
struct incoming_connection_alert;
struct session_stats_alert;
struct dht_error_alert;
struct session_stop_over_alert;
struct dht_immutable_item_alert;
struct dht_mutable_item_alert;
struct dht_put_alert;
struct dht_outgoing_get_peers_alert;
struct log_alert;
struct dht_lookup;
struct dht_stats_alert;
struct dht_log_alert;
struct dht_pkt_alert;
struct dht_get_peers_reply_alert;
struct dht_direct_response_alert;
struct picker_log_alert;
struct session_error_alert;
struct dht_live_nodes_alert;
struct session_stats_header_alert;
struct dht_sample_infohashes_alert;
struct alerts_dropped_alert;
struct socks5_alert;
struct communication_new_device_id_alert;
struct communication_new_message_alert;
struct communication_confirmation_root_alert;
struct communication_syncing_message_alert;
struct communication_friend_info_alert;
struct communication_log_alert;
struct communication_last_seen_alert;
struct blockchain_log_alert;
struct blockchain_new_head_block_alert;
struct blockchain_new_tail_block_alert;
struct blockchain_new_consensus_point_block_alert;
struct blockchain_rollback_block_alert;
struct blockchain_fork_point_block_alert;
struct blockchain_top_three_votes_alert;
struct blockchain_new_transaction_alert;
struct blockchain_state_alert;
struct blockchain_syncing_block_alert;
struct blockchain_syncing_head_block_alert;
struct blockchain_tx_confirmation_alert;
struct referred_status_alert;
struct communication_message_arrived_alert;
struct blockchain_tx_sent_alert;
struct blockchain_tx_arrived_alert;
struct communication_user_info_alert;
struct communication_user_event_alert;
struct blockchain_state_array_alert;
struct blockchain_fail_to_get_chain_data_alert;
struct blockchain_online_peer_alert;
struct communication_peer_attention_alert;
struct transport_log_alert;
struct assemble_log_alert;

// include/ip2/announce_entry.hpp
TORRENT_VERSION_NAMESPACE_2
struct announce_infohash;
struct announce_endpoint;
struct announce_entry;
TORRENT_VERSION_NAMESPACE_2_END

// include/ip2/bdecode.hpp
struct bdecode_node;

// include/ip2/bitfield.hpp
struct bitfield;

// include/ip2/client_data.hpp
struct client_data_t;

// include/ip2/disk_buffer_holder.hpp
struct buffer_allocator_interface;
struct disk_buffer_holder;

// include/ip2/entry.hpp
struct entry;

// include/ip2/error_code.hpp
struct storage_error;

// include/ip2/extensions.hpp
struct plugin;
struct torrent_plugin;
struct peer_plugin;
struct crypto_plugin;

// include/ip2/hasher.hpp
class hasher;
class hasher256;

// include/ip2/info_hash.hpp
struct info_hash_t;

// include/ip2/ip_filter.hpp
struct ip_filter;
class port_filter;

// include/ip2/kademlia/dht_state.hpp
namespace dht {
struct dht_state;
}

// include/ip2/kademlia/dht_storage.hpp
namespace dht {
struct dht_storage_counters;
}
namespace dht {
struct dht_storage_interface;
}

// include/ip2/peer_class.hpp
struct peer_class_info;

// include/ip2/peer_class_type_filter.hpp
struct peer_class_type_filter;

// include/ip2/peer_info.hpp
TORRENT_VERSION_NAMESPACE_2
struct peer_info;
TORRENT_VERSION_NAMESPACE_2_END

// include/ip2/peer_request.hpp
struct peer_request;

// include/ip2/performance_counters.hpp
struct counters;

// include/ip2/session.hpp
struct session_proxy;
struct session;

// include/ip2/session_handle.hpp
struct session_handle;

// include/ip2/session_params.hpp
struct session_params;

// include/ip2/session_stats.hpp
struct stats_metric;

// include/ip2/settings_pack.hpp
struct settings_interface;
struct settings_pack;
}

namespace lt = ip2;

#endif // TORRENT_FWD_HPP
