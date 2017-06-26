#!/bin/bash

set -eu

ARGS=""
for i in "$@"; do 
    i="${i//\\/\\\\}"
    ARGS="$ARGS \"${i//\"/\\\"}\""
done

CAMERA=${CAMERA:-/dev/video0}

if [ -e $CAMERA ]; then
  GID=`stat -c %g $CAMERA`
  CAMERA_DEVICE="--device $CAMERA"
else
  GID=`id -g`
  CAMERA_DEVICE=
fi

docker run \
    --net host \
    -ti \
    --rm \
    $CAMERA_DEVICE \
    -e CAMERA=$CAMERA \
    -e OMP_NUM_THREADS=1 \
    -v `pwd`:/mnt \
    -v `pwd`/../rpc:/mnt/.rpc \
    -u `id -u`:$GID \
    bitclave/camera \
    sh -c "make -j`nproc` && ./camera $ARGS"
