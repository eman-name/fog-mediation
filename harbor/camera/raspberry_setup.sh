#!/bin/sh

set -eu

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root"
   exit 1
fi

# update system
apt-get update
apt-get upgrade -y

# disable hdmi output and enable camera
cat <<EOT >> /boot/config.txt
hdmi_blanking=2
gpu_mem=128
start_x=1
EOT

# install mjpg-streamer
apt-get install -y cmake libjpeg8 libjpeg8-dev
curl -sfL https://github.com/jacksonliam/mjpg-streamer/archive/master.tar.gz | tar zxf -
cd mjpg-streamer-master/mjpg-streamer-experimental/
make
mkdir -p /usr/share/mjpg-streamer
mv www /usr/share/mjpg-streamer/www
cp mjpg_streamer /usr/bin/
cp input_raspicam.so output_http.so /usr/lib/
cd ../..
rm -rf mjpg-streamer-master
apt-get remove -y cmake libjpeg8-dev
apt-get autoremove -y
apt-get clean -y

# make mjpg-streamer start at boot
sed -i '$ d' /etc/rc.local
cat <<EOT >> /etc/rc.local
mjpg_streamer -i "input_raspicam.so -rot 90 -x 1280 -y 720" -o "output_http.so -w /usr/share/mjpg-streamer/www -c pi:bitclave -n" &
exit 0
EOT

echo Done!
