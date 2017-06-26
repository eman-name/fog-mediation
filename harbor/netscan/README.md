### NETSCAN ###

This Docker app is a wrapper for [kismet_server](https://www.kismetwireless.net/download.shtml). This service shows all 802.11 wireless devices in proximity of a capture source including APs (i.e., Wi-Fi routers) and user devices (e.g., mobile phones, tablets, and notebooks).

#Setup

Build docker image with:

    $ docker build --rm -t bitclave/netscan .

### Specify Source

For the purposes of the demo, you should really only have to (1) set a capture source and (2) specify an inclusive filter for the APs of interests. 

Additional details on the `kismet.conf` file are available within the file itself as well as on the [Kismet Wireless Documentation](https://www.kismetwireless.net/documentation.shtml) web page.

### Set Capture Source

By default `run.sh` will use `wlan0` as wifi network interface. If you want different one pass it to the argument.

### Filtering APs

By default `kismet_server` will log all APs and devices within range of the capture source(s). Kismet allows networks to be excluded from logging based on BSSID or MAC address. 

For the purposes of the demo, it's useful to limit logging to only APs of interest to reduce clutter in the GUI.See the section *11. Filtering* in the [Kismet Wireless Documentation] for details on how to specify inclusive or exclusive network filters.

# Viewing captured data

Connect to kismet:

    $ telnet localhost 2501

And execute:

    !0 REMOVE TIME
    !0 ENABLE CLIENT *

Format of `CLIENT` data is descibed with:

    !0 CAPABILITY CLIENT

# References
* [Kismet Wireless Documentation](https://www.kismetwireless.net/documentation.shtml)
