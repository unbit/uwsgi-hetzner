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

legion = failover 3.3.3.3:2000 9998 bf-cbc:secret1
legion-node = failover 1.1.1.1:2000
legion-node = failover 2.2.2.2:2000
legion-lord = failover hetzner-failoverip:username=FOO,password=BAR,ip=4.4.4.4,active_ip=3.3.3.3
legion-death-on-lord-error = 300
legion-tolerance = 60
```

The configuration is a bit verbose (expecially if you are used to multicast legion setup) but should be easy understandable
