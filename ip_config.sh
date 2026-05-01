#!/usr/bin/bash 

ip tuntap add mode tap name tap0
ip addr add 172.16.100.1/24 dev tap0
ip link add link tap0 name vlan1 type vlan id 1
ip addr add 172.16.101.1/24 dev vlan1
ip link set tap0 up
ip link set vlan1 up
