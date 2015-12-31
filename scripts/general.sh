#!/bin/bash

ping -q -c1 -W1 192.168.1.40 > /dev/null
dev1=$?
ping -q -c1 -W1 192.168.1.45 > /dev/null
dev2=$?

if [ $dev1 -eq 0 ] || [ $dev2 -eq 0 ] ;
then
	sudo /etc/init.d/vod stop
else
	sudo /etc/init.d/vod status
fi
