#!/bin/bash

# .............................................
INTERFACE="eth0"          # ........................ eth0, enp0s3, ens33 ...
IP_ADDR="192.168.0.22"
NETMASK="255.255.255.0"
GATEWAY="192.168.0.200"

# ............... root ............
if [ "$EUID" -ne 0 ]; then
echo "......... root .................................sudo $0..."
exit 1
fi

# ........................
if ! ip link show "$INTERFACE" > /dev/null 2>&1; then
echo "............... $INTERFACE ........."
exit 2
fi

echo "............... $INTERFACE ............ IP..."

# ...... IP ..................ifconfig ............... IP...
ifconfig "$INTERFACE" "$IP_ADDR" netmask "$NETMASK" up
if [ $? -ne 0 ]; then
echo "ifconfig ............"
exit 3
fi

# .............................................
route del default > /dev/null 2>&1

# ........................
route add default gw "$GATEWAY"
if [ $? -ne 0 ]; then
echo "........................"
exit 4
fi

echo "..............."
echo "IP .........$IP_ADDR"
echo "...............$NETMASK"
echo ".........$GATEWAY"
echo ""
echo "....................."
ifconfig "$INTERFACE" | grep -E "inet |netmask |broadcast "
route -n | grep -E "UG|Destination"
