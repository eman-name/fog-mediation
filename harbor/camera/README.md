Setup
=====

Refer to [harbor](https://bitbucket.org/bitclave1/harbor) repository for docker installation instructions.

If you have not run `docker-compose` yet, then you'll need to build `bitclave/camera` docker image. Do it like this:

    $ docker build --rm -t bitclave/camera .

Running
=======

`run.sh` script accepts different commands:

* `run.sh stream http://<CAMERA-URL> http://<POST-URL>` - will start analyzing images from network camera and post JSON results to second URL
* `run.sh load Martins` - will start capturing images from connected camera and assign label `Martins` to them
* `run.sh load Martins folder/` - will read *.jpg files from `folder` (currently must be inside same folder as script) and assign label `Martins` to them
* `run.sh test` - captures one image from connected camera and tries to recognize person(s)
* `run.sh test file.jpg` - reads file.jpg (currently must be inside same folder as script) and tries to recognize person(s)

`load` command updates `faces.db` file, no images are stored there, only some floats. These floats are uses to train classifier model used by `test` and `stream` commands to recognize person faces. Multiple faces can be detected in image.

`test` command prints out in output recognized faces and confidence factor (0 to 1). It also produces `output*.jpg` files as debugging info.

By default camera uses /dev/video0 device, which is default camera device. To specify different camera device use `CAMERA` environment variable. For example:

    $ CAMERA=/dev/video1 ./run.sh <arguments>

You can also use http stream to try `test` command:

    $ CAMERA=http://<CAMERA-URL> ./run.sh test

Or you can use locally connected camera to for `stream` command:

    $ ./run.sh stream /dev/video0 http://<POST-URL>

Setting up Raspberry Pi with camera
===================================

1. download Raspbian Jessie Lite image from https://www.raspberrypi.org/downloads/raspbian/

2. write image to sdcard, and mount both sdcard partitions

3. create empty file at `<partition1>/ssh` location

4. if wifi will be used instead of ethernet then edit `<partition2>/etc/wpa_supplicant/wpa_supplicant.conf` file and append wifi network information:

        network={
	        ssid="<SSID>"
	        psk="<PASSWORD>"
	        #key_mgmt=NONE
	        #scan_ssid=1
        }

    If you have open wifi access point, comment out `psk` and uncomment `key_mgmt`. If SSID is hidden then uncomment `scan_ssid` line.

5. by default Pi will use dhcp for wifi to get IP address, if you want static address, edit `<partition2>/etc/network/interfaces` file and change following:

        iface wlan0 inet manual
            wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf

    to

        iface wlan0 inet static
            address 192.168.1.20
            netmask 255.255.255.0
            gateway 192.168.1.1
            wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf

    For ethernet network change `eth0` instead of `wlan0` section. Use either `dhcp` or `static` after `inet` keyword.

6. unmount partitions, plug in the power, and connect with ssh - `ssh pi@<IP>`, default password is `raspberry`

7. change password with `passwd` and optionally add ssh public key

8. set correct timezone `sudo timedatectl set-timezone America/Los_Angeles`

9. copy `raspberry_setup.sh` script and execute it with sudo, it will:

    * update packages
    * disable hdmi output
    * enable camera
    * install [mjpg-streamer](https://github.com/jacksonliam/mjpg-streamer) and make it run at boot
    * mjpg-streamer will capture video with 1280x720 resolution and 1 frame per second

10. do the reboot - `sudo reboot`

11. open `http://<IP>:8080/stream_simple.html` in browser to see camera output

    * username is `pi`
    * password is `bitclave`

Use `http://pi:bitclave@<IP>:8080/?action=stream` URL to access camera video stream directly.

Streaming local webcamera over http
===================================

First make sure you have opencv with python support installed. For macOS do the following:

1. Install Xcode, and make sure you have command-line tools installed (run `xcode-select --install`)
2. Install [Homebrew](https://brew.sh/)
3. Run `brew tap homebrew/science`
4. Run `brew install opencv python`

Launch `local_stream.py` script to serve captured stream. On macOS you need to execute it like this:

    $ /usr/local/bin/python local_stream.py

Now try accessing http://localhost:9999/index.html in a browser. You should see captured stream.

Then you can use `CAMERA=http://localhost:9999/cam.mjpg` as direct feed for camera.

Misc
====

Here are some datasets with people faces:

* [Labeled Faces in the Wild](http://vis-www.cs.umass.edu/lfw/)
* [ChokePoint Dataset](http://arma.sourceforge.net/chokepoint/)
* [PubFig: Public Figures Face Database](http://www.cs.columbia.edu/CAVE/databases/pubfig/)
* [Unconstrained Facial Images](http://ufi.kiv.zcu.cz/)
