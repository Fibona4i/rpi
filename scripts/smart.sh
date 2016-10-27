#!/bin/bash
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/local/games:/usr/games
PFX=/usr/local
export PATH=$PATH:$PFX/bin
export LD_RUN_PATH=$LD_RUN_PATH:$PFX/lib:$PFX/lib/gstreamer-1.0:/opt/vc/lib:/opt/vc/lib/plugins/plugins
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PFX/lib:$PFX/lib/gstreamer-1.0:/opt/vc/lib:/opt/vc/lib/plugins/plugins
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PFX/lib/pkgconfig
export GST_PLUGIN_PATH=$PFX/lib/gstreamer-1.0:$PFX/lib
export GST_OMX_CONFIG_DIR=/usr/local/etc/xdg/
#export GST_DEBUG=*:4
#export GST_DEBUG_FILE=/home/pi/smart/gst_deb.log
#env > /tmp/env2.output

killall -9 vlc gst-launch-1.0 smarthome ping.sh

until /home/pi/smart/smarthome /home/pi/smart/config.ini ; do
    echo "Server 'smart' crashed with exit code $?. $(date) Respawning.." >> /home/pi/smart/debug.txt
    killall -9 vlc gst-launch-1.0 smarthome ping.sh
    sleep 1
done
