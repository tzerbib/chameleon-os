#!/usr/bin/bash 

ip link set "vlan$1" down
ip link add link tap0 name "vlan$2" type vlan id "$2"
ip addr add 172.16.101.1/24 dev "vlan$2"
ip link set "vlan$2" up
