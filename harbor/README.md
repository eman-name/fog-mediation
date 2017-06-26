Overview
========

``` sh
fog-mediation
|
|- harbor
	|
	|-- control (web server container)
	|
	|-- camera (face recognition container)
	|
	|-- netscan (passive scan container)
	|
	*
```

This project is harbor for all the docker containers.

It allows to start up the control, camera, and netscan containers.

Setup
=====

First install [docker](https://docs.docker.com/engine/installation/) and [docker-compose](https://docs.docker.com/compose/install/) on your system.

For Mac users: you need to install Ubuntu in virtualbox or in VMware Fusion. In Fusion: you might need to increase the number of cores to 2 in order to compile OpneBLAS. Follow instructions from http://askubuntu.com/questions/840822/ubuntu-16-10-doesnt-work-in-virtual-machine-vmware/841283#841283 to fix the vm crash issue.

Then prepare environemnt variables (do it only once):

    $ echo HOST_UID=$UID > docker-compose.env
    $ echo HOST_WIFI=wlan0 >> docker-compose.env

Replace `wlan0` with your wifi interface which will be used to capture wireless device signal strength.

Running
=======

Execute the following commands to get control webserver and the camera stream running

To use your laptop webcam run: 
	$ /usr/local/bin/python camera/local_stream.py

Now try accessing the camera feed at http://localhost:9999/cam.mjpg in your web browser.

To launch the control and camera service run:
	
	$ CAMERA=http://127.0.0.1:9999/cam.mjpg docker-compose up

	Add `--build` if running first time, or you have modified `Dockerfile` for any container.
	Append `-d` to have it run in background.

Now try accessing http://localhost:8080 in your web browser.

To remove services run:

    $ docker-compose down

Docker tips & tricks
====================

Remove all stopped docker containers:

    $ docker rm `docker ps -a | awk '{ print($1); }'`

Remove all unused docker images:

    $ docker rmi `docker images | grep "<none>" | awk '{ print($3); }'`
