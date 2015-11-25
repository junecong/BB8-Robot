#!/bin/bash          
if systemctl status wpa_supplicant | grep inactive
then
	echo "inactive"
	systemctl start wpa_supplicant
	systemctl status wpa_supplicant
else
	echo "active"
fi

if wpa_cli -iwlan0 interface | grep p2p-dev-wlan0
then
	wpa_cli -ip2p-dev-wlan0 p2p_find
	while [ -z $mac ]
	do
		sleep 5
		mac="$(wpa_cli -ip2p-dev-wlan0 p2p_peers)"
		echo $mac
	done
else
	echo "error: p2p-dev-wlan0 not found"
fi

wpa_cli -ip2p-dev-wlan0 p2p_group_add
if wpa_cli -ip2p-dev-wlan0 interface | grep p2p-wlan0-*
then
	var="$(wpa_cli -ip2p-dev-wlan0 interface | grep p2p-wlan0-*)"
	pin="$(wpa_cli -i$var wps_pin any)"
	echo "Pin: $pin"
else
	echo "error: no p2p-wlan0-*"
fi

echo "Press any button to continue:"
read var

echo "IP address:"
ifconfig p2p-wlan0-0 | grep "inet " | awk -F'[: ]+' '{ print $4 }'
