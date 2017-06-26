#!/bin/sh

if (( $# == 1 )); then
    WIFI=$1
else
    WIFI=wlan0
fi

echo "**** Using $WIFI as wifi interface"

docker run \
    -ti \
    --rm \
    --net host \
    --privileged \
    -v `pwd`/kismet.conf:/mnt/kismet.conf \
    bitclave/netscan \
    kismet_server -f /mnt/kismet.conf -n -s -c $WIFI
