/*

Copyright (c) 2015, Steven Siloti
Copyright (c) 2015-2020, Arvid Norberg
Copyright (c) 2016, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include "test.hpp"

#if !defined TORRENT_DISABLE_EXTENSIONS && !defined TORRENT_DISABLE_DHT

#include "ip2/config.hpp"
#include "ip2/session.hpp"
#include "ip2/session_params.hpp"
#include "ip2/extensions.hpp"
#include "ip2/alert_types.hpp"
#include "ip2/bdecode.hpp"
#include "setup_transfer.hpp"

using namespace lt;

namespace
{

struct test_plugin : plugin
{
	feature_flags_t implemented_features() override
	{
		return plugin::dht_request_feature;
	}

	bool on_dht_request(string_view /* query */
		, udp::endpoint const& /* source */, bdecode_node const& message
		, entry& response) override
	{
		if (message.dict_find_string_value("q") == "test_good")
		{
			response["r"]["good"] = 1;
			return true;
		}
		return false;
	}
};

dht_direct_response_alert* get_direct_response(lt::session& ses)
{
	for (;;)
	{
		alert* a = ses.wait_for_alert(seconds(30));
		// it shouldn't take more than 30 seconds to get a response
		// so fail the test and bail out if we don't get an alert in that time
		TEST_CHECK(a);
		if (!a) return nullptr;
		std::vector<alert*> alerts;
		ses.pop_alerts(&alerts);
		for (std::vector<alert*>::iterator i = alerts.begin(); i != alerts.end(); ++i)
		{
			if ((*i)->type() == dht_direct_response_alert::alert_type)
				return static_cast<dht_direct_response_alert*>(&**i);
		}
	}
}

}

#endif // #if !defined TORRENT_DISABLE_EXTENSIONS && !defined TORRENT_DISABLE_DHT

TORRENT_TEST(direct_dht_request)
{
#if !defined TORRENT_DISABLE_EXTENSIONS && !defined TORRENT_DISABLE_DHT

	std::vector<lt::session_proxy> abort;
	settings_pack sp;
	sp.set_bool(settings_pack::enable_lsd, false);
	sp.set_bool(settings_pack::enable_natpmp, false);
	sp.set_bool(settings_pack::enable_upnp, false);
	sp.set_str(settings_pack::dht_bootstrap_nodes, "");
	sp.set_int(settings_pack::max_retry_port_bind, 800);
	sp.set_str(settings_pack::listen_interfaces, "127.0.0.1:42434");
	lt::session responder(session_params(sp, {}));
	sp.set_str(settings_pack::listen_interfaces, "127.0.0.1:45434");
	lt::session requester(session_params(sp, {}));

	responder.add_extension(std::make_shared<test_plugin>());

	// successful request

	entry r;
	r["q"] = "test_good";
	requester.dht_direct_request(uep("127.0.0.1", responder.listen_port())
		, r, client_data_t(reinterpret_cast<int*>(12345)));

	dht_direct_response_alert* ra = get_direct_response(requester);
	TEST_CHECK(ra);
	if (ra)
	{
		bdecode_node response = ra->response();
		TEST_EQUAL(ra->endpoint.address(), make_address("127.0.0.1"));
		TEST_EQUAL(ra->endpoint.port(), responder.listen_port());
		TEST_EQUAL(response.type(), bdecode_node::dict_t);
		TEST_EQUAL(response.dict_find_dict("r").dict_find_int_value("good"), 1);
		TEST_EQUAL(ra->userdata.get<int>(), reinterpret_cast<int*>(12345));
	}

	// failed request

	requester.dht_direct_request(uep("127.0.0.1", 53545)
		, r, client_data_t(reinterpret_cast<int*>(123456)));

	ra = get_direct_response(requester);
	TEST_CHECK(ra);
	if (ra)
	{
		TEST_EQUAL(ra->endpoint.address(), make_address("127.0.0.1"));
		TEST_EQUAL(ra->endpoint.port(), 53545);
		TEST_EQUAL(ra->response().type(), bdecode_node::none_t);
		TEST_EQUAL(ra->userdata.get<int>(), reinterpret_cast<int*>(123456));
	}

	abort.emplace_back(responder.abort());
	abort.emplace_back(requester.abort());
#endif // #if !defined TORRENT_DISABLE_EXTENSIONS && !defined TORRENT_DISABLE_DHT
}
