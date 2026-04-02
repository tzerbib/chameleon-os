#!/usr/bin/bash 

ip link set vlan0 down
ip link add link tap0 name vlan1 type vlan id 2
ip addr add 172.16.101.1/24 dev vlan1
ip link set vlan1 up
