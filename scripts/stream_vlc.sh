#!/bin/bash
# /etc/init.d/mjpg_streamer.sh
### BEGIN INIT INFO
# Provides:          stream.sh
# Required-Start:    $networking
# Required-Stop:     $networking
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: vlc_streamer for webcam
# Description:       Start VoD RTSP Server
### END INIT INFO
f_message(){
        echo "[+] $1"
}

# Carry out specific functions when asked to by the system
case "$1" in
        start)
                f_message "Starting vlc_streamer"
		/usr/bin/cvlc v4l2:///dev/video0 :input-slave=alsa://hw:1,0 :v4l2-input=0 :v4l2-width=320 :v4l2-height=240 :v4l2-aspect-ratio=4\:3 :v4l2-fps=15 :v4l2-use-libv4l2 :v4l2-tuner=0 :v4l2-tuner-frequency=22050 :v4l2-tuner-audio-mode=0 :live-caching=500 --sout '#transcode{vcodec=WMV2,vb=512,fps=5,scale=1,width=320,height=240,acodec=s16l,ab=32,channels=1,samplerate=22050}:http{dst=:8080/stream.wmv}' :sout-keep > /dev/null 2>&1 &                
		sleep 2
                f_message "vlc_streamer started"
                ;;
        stop)
                f_message "Stopping vlc_streamer…"
                killall vlc
                f_message "vlc_streamer stopped"
                ;;
        restart)
                f_message "Restarting daemon: vlc_streamer"
                killall vlc
		sleep 1
		/usr/bin/cvlc v4l2:///dev/video0 :input-slave=alsa://hw:1,0 :v4l2-input=0 :v4l2-width=320 :v4l2-height=240 :v4l2-aspect-ratio=4\:3 :v4l2-fps=15 :v4l2-use-libv4l2 :v4l2-tuner=0 :v4l2-tuner-frequency=22050 :v4l2-tuner-audio-mode=0 :live-caching=500 --sout '#transcode{vcodec=WMV2,vb=512,fps=5,scale=1,width=320,height=240,acodec=s16l,ab=32,channels=1,samplerate=22050}:http{dst=:8080/stream.wmv}' :sout-keep > /dev/null 2>&1 &
                sleep 2
                f_message "Restarted daemon: vlc_streamer"
                ;;
        status)
                pid=`ps -A | grep vlc | grep -v "grep" | grep -v vlc. | awk '{print $1}' | head -n 1`
                if [ -n "$pid" ];
                then
                        f_message "vlc_streamer is running with pid ${pid}"
                        f_message "vlc_streamer was started with the following command line"
                        cat /proc/${pid}/cmdline ; echo ""
                else
                        f_message "Could not find vlc_streamer running"
                fi
                ;;
        *)
                f_message "Usage: $0 {start|stop|status|restart}"
                exit 1
                ;;
esac

exit 0
