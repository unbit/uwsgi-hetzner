uwsgi-hetzner
=============
A uWSGI plugin for integration with hetzner's services (http://www.hetzner.de/en/)

This is the plugin used by uwsgi.it service in the hetzner hardware infrastructure.

Currently it only exposes a Legion action for ip takeover.

Basically when a node in a Legion become the Lord it will call the hetzner api to route requests for the failover ip to the lord address.

As we cannot rely on multicast on hetzner network we need to manually add every node joining the cluster.

Supposing you have 3 servers: 1.1.1.1, 2.2.2.2, 3.3.3.3 and failover ip: 4.4.4.4

you can configure the legion in this way:

server 1.1.1.1

```ini
[uwsgi]
plugin = hetzner

legion = failover 1.1.1.1:2000 9999 bf-cbc:secret1
legion-node = failover 2.2.2.2:2000
legion-node = failover 3.3.3.3:2000
legion-lord = failover hetzner-failoverip:username=FOO,password=BAR,ip=4.4.4.4,active_ip=1.1.1.1
legion-death-on-lord-error = 300
legion-tolerance = 60
```

server 2.2.2.2

```ini
[uwsgi]
plugin = hetzner

legion = failover 2.2.2.2:2000 9998 bf-cbc:secret1
legion-node = failover 1.1.1.1:2000
legion-node = failover 3.3.3.3:2000
legion-lord = failover hetzner-failoverip:username=FOO,password=BAR,ip=4.4.4.4,active_ip=2.2.2.2
legion-death-on-lord-error = 300
legion-tolerance = 60
```

server 3.3.3.3

```ini
[uwsgi]
plugin = hetzner

legion = failover 3.3.3.3:2000 9997 bf-cbc:secret1
legion-node = failover 1.1.1.1:2000
legion-node = failover 2.2.2.2:2000
legion-lord = failover hetzner-failoverip:username=FOO,password=BAR,ip=4.4.4.4,active_ip=3.3.3.3
legion-death-on-lord-error = 300
legion-tolerance = 60
```

The configuration is a bit verbose (expecially if you are used to multicast legion setup) but should be easy understandable.

Each node has different priority (from 9999 to 9997)

The lord action is the 'hetzner-failoverip' one, exposed by the 'hetzner' plugin. This is a keyval based option:

``username`` web api username (required)

``password`` web api password (required)

``url`` alternative api url

``timeout`` timeout (default 60 seconds)

``ip`` failover ip (required)

``active_ip`` ip of the instance getting the failover routing (required)

``ssl_no_verify`` do not verify ssl certificate of the api url


Always use the ``legion-tolerance = 60`` option to avoid mess as the web api requires up to 30 seconds to move an ip.

``legion-death-on-lord-error`` is only available from uWSGI 2.0.1 and allows you to suspend a legion member if it fails during the attempt to run the "lord action". (read: if the api call fails, another member can try to became the lord). The value is the amount of seconds to wait before un-suspending the instance.

The plugin is 2.0 friendly, so you can build it with

```
uwsgi --build-plugin uwsgi-hetzner
```

Security notes
--------------

Try to limit the api access to the ip addresses of your instances.

Use your system firewall to protect access to the legion ports from unallowed machine (the secret key should protect you, even from reply attacks, but obviously can be stolen)
