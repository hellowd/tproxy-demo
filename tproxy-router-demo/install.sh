#!/bin/bash

ip netns add clt
ip netns add srv
ip netns add rt

ip link add clt-inner type veth peer name clt-outer
ip link add srv-inner type veth peer name srv-outer
ip link set clt-inner netns clt
ip link set clt-outer netns rt
ip link set srv-inner netns srv
ip link set srv-outer netns rt


ip netns exec clt ip addr add 192.168.100.10/24 dev clt-inner
ip netns exec rt ip addr add 192.168.100.1/24 dev clt-outer
ip netns exec srv ip addr add 192.168.200.10/24 dev srv-inner
ip netns exec rt ip addr add 192.168.200.1/24 dev srv-outer

ip netns exec clt ip link set clt-inner up
ip netns exec rt ip link set clt-outer up
ip netns exec srv ip link set srv-inner up
ip netns exec rt ip link set srv-outer up
ip netns exec rt ip link set lo up


ip netns exec clt ip route add default via 192.168.100.1
ip netns exec srv ip route add default via 192.168.200.1

ip netns exec rt iptables -t mangle -A PREROUTING -i clt-outer -p tcp -m tcp --dport 80 -j TPROXY --on-port 50080 --tproxy-mark 1/1
ip netns exec rt iptables -t mangle -A PREROUTING -i srv-outer -p tcp -m tcp --sport 80 -j MARK --set-mark 1/1
ip netns exec rt ip rule add fwmark 1/1 table 100
ip netns exec rt ip route add local 0.0.0.0/0 dev lo table 100

ip netns exec rt sysctl -w net.ipv4.ip_forward=1

