/*

Copyright (c) 2018, 2020, Arvid Norberg
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_GENERATE_PEER_ID_HPP_INCLUDED
#define TORRENT_GENERATE_PEER_ID_HPP_INCLUDED

#include "ip2/peer_id.hpp"
#include "ip2/aux_/export.hpp"

namespace ip2 { namespace aux {

struct session_settings;

TORRENT_EXTRA_EXPORT peer_id generate_peer_id(session_settings const& sett);

}}

#endif

