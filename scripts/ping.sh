#!/bin/bash

ping -q -c1 -W1 192.168.1.140 > /dev/null
dev1=$?
ping -q -c1 -W1 192.168.1.145 > /dev/null
dev2=$?

f_message(){
        echo "[+] $1"
}

check_smart_status(){
        pid_smart=`ps -A | grep smart.sh | grep -v "grep" | grep -v vlc. | awk '{print $1}' | head -n 1`
	echo $pid_smart;

        if [ -n "$pid_smart" ] ;
        then
                f_message "streamer is running with pid ${pid_smart}"
                #cat /proc/${pid_smart}/cmdline ; echo ""
		return 1
        else
		echo "ERROR! Restarting smart.sh service..."
		return 0
        fi
}

_evalBg() {
    eval "$@" &>/dev/null &disown;
}

if check_smart_status ;
then
	if [ $dev1 -eq 1 ] && [ $dev2 -eq 1 ] ;
	then
		echo "smart.sh is running"
		/home/pi/smart/smart.sh &
		#_evalBg "/home/pi/smart/smart.sh"
	fi
else
	if [ $dev1 -eq 0 ] || [ $dev2 -eq 0 ] ;
	then
		echo "smart.sh kill -55"
		killall -55 smarthome
	fi
fi

#if [ $dev1 -eq 0 ] || [ $dev2 -eq 0 ] ;
#then
#	killall -55 smarthome
#else
#	/home/pi/smart/smart.sh
#fi
