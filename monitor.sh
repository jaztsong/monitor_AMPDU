#! /bin/bash

if [ "$1" == "-h" ]; then
        echo "Usage: `basename $0`
        Please input the input as the following order:
        Location, ex. Tent or Gate
        Interface, ex. wlan0, wlan1
        Frequency, ex. 5 for 5GHz
        Channel, ex. channel number 1,11,153"
        exit 0 
fi
loc=$1
in=$2
fre=$3
chan=$4
dat=`date +"%Y-%m-%d"`
tim=`date +"%H:%M"`
sudo ifconfig $in down
sudo iwconfig $in mode monitor
sudo ifconfig $in up
sudo iwconfig $in chan $chan
# sudo tcpdump -i $in -n -w $loc-$dat-$tim-$fre-$chan.pcap
