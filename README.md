## IP2
 IP2(Internet Protocol 2) is a p2p(peer to peer), decentralized and server-less networking protocol based on UDP. It is designed for restriction free "public key to public key" overlay communication on UDP. IP2 uses self-generated 256 bits public key as address, while previous Internet Protocol RFC 791 uses hierarchically assigned address ether IPv4 or IPv6 to communicate. And importantly IP2 is not easily affacted by network devices such as router, firewall and NAT. So it can adapt to IOT network very well.

## Roadmap

### [libTAU](https://github.com/Tau-Coin/libTAU)

 libTAU is forked from [libtorrent](https://github.com/arvidn/libtorrent). libtorrent uses ip address as node id for DHT network. Differently, libTAU uses public key as its node id, so the nodes with the nearest public key automatically compose a "captrue swarm". By "captrue swarm" a node can be located very easily.

### IP2

 IP2 is forked from [libTAU](https://github.com/Tau-Coin/libTAU). Compared with libTAU, BLOB(Binary Large Object) can be transfered.

## Building

 See [building.md](https://github.com/Tau-Coin/IP2/blob/main/docs/building.md) for more details on how to build and install.

## Usage

### API
 
 See [API](https://github.com/Tau-Coin/IP2/blob/main/docs/API.txt) for more details on how to call ip2 API.

### Example

#### 1. launch ip2 session

&emsp;&emsp;settings_pack sp_set;
&emsp;&emsp;sp_set.set_str(settings_pack::dht_bootstrap_nodes, bootstrap_nodes);
&emsp;&emsp; // bootstrap nodes format: tau:://public_key@ip:port
&emsp;&emsp; // tau://9CB3E5A7B060D06A6A47E0E9AFCE4455077B0F5B313E7FF105A48A6D7AF356C7@10.0.2.15:6881
&emsp;&emsp;sp_set.set_str(settings_pack::listen_interfaces, listen_interfaces.str());
&emsp;&emsp; // listen interface format: 192.168.2.120:6881
&emsp;&emsp;session_params sp_param(sp_set);
&emsp;&emsp;session ses(sp_param);

#### 2. dispatch alerts

&emsp;&emsp;std::vector<alert*> alert_queue;
&emsp;&emsp;bool quit = false;
&emsp;&emsp;while (!quit)
&emsp;&emsp;{
&emsp;&emsp;&emsp;&emsp;ses.pop_alerts(&alert_queue);
&emsp;&emsp;&emsp;&emsp;for(std::vector<alert*>::iterator i = alert_queue.begin()
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; , end(alert_queue.end()); i != end; ++i)
&emsp;&emsp;&emsp;&emsp;{
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;int alert_type = (*i)->type();
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;switch(alert_type){
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;case put_data_alert::alert_type:// put blob done
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;case relay_data_uri_alert::alert_type: // relay uri done
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;case incoming_relay_data_uri_alert::alert_type:// incoming relay uri
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;case get_data_alert::alert_type:// get blob done
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;case relay_message_alert::alert_type: // relay message done
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;case incoming_relay_message_alert::alert_type: // incoming relay message
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;}
&emsp;&emsp;&emsp;&emsp;}
&emsp;&emsp;}

#### 3. call IP2 API

&emsp;&emsp;error_code ec = ses.put_data_into_swarm(blob, uri);
&emsp;&emsp;error_code ec = ses.relay_data_uri(receiver, uri, timestamp);
&emsp;&emsp;error_code ec = ses.get_data_from_swarm(sender, uri, timestamp);
&emsp;&emsp;error_code ec = ses.relay_message(receiver, message);

 For more detail, please see [ip2-shell](https://github.com/Tau-Coin/ip2-shell).

## Contribute
 Now libip2 is just alpha version. If any problems, please help us with the open issues.
