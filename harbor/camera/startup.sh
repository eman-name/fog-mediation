#!/bin/sh

set -eu

if [ -z "$CAMERA" ];
then
  echo "*** ERROR !!!"
  echo "*** Please specify URL for camera in CAMERA environment variable"
  exit 1
fi

CORES=`cat /proc/cpuinfo | grep processor | wc -l`
export OMP_NUM_THREADS=1

cd /mnt
sudo -E -u \#$HOST_UID make -j$CORES
sudo -E -u \#$HOST_UID ./camera stream $CAMERA $CONTROL
