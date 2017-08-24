##Instructions on set up:

The main purpose is to install Openface on both sides, and mount folders which including *client.py* for client side, and *handler.py*, *server.py* for server side.

1. Change the IP address for $HOST in *client.py*, line 24 to your cloud IP address.

2. On server, you should already have Docker installed, 
and then Openface project pulled out by: `docker pull bamos/openface`

3. Upload server.py, handler.py to any legitimate cloud server folder that you have full access.
Let's say `$HOME` for now.

4. On server, run `docker run  -p 60000:60000 -it -v $HOME:/research bamos/openface /bin/bash`, to mount $HOME foler, and open port 60000.

5. On local machine, do the same thing to mount folder that include client.py, and open port 60000. For example, if `$HOME` has *client.py*, then `sudo docker run -p 60000:60000 -it -v $HOME:/research bamos/openface /bin/bash`. Noted we need root privilege because we need *scp* to transfer aligned images later.

## Instructions on running:

1. Go to the folder which including *handler.py*, *server.py*, then `./handler.py` to set up listening on server.

2. Execution on client machine
  * For training, you must have a folder which include sub-folders of images for each person. See example in *classify-test* folder, or the training process won't work properly. Then `./client.py classify-test/raw/ train` as an example for training images in *classify-test/raw*. This will create a *label* folder which contains model (classifier.pkl) that you can use for prediction purpose.
  
  * For classification, you can simple have a folder of images, like *img* or *classify-test/raw*, and then `./client.py classify-test/raw/ classify`. This example use the default model which locate in */root/openface/models/openface/celeb-classifier.nn4.small2.v1.pkl* to predict, but you can indicate our trained model by `./client.py classify-test/raw/ classify $trained_model` Also, this process will create *alignedImg* folder by default (you can change this as well), and upload it to the server to do classification. The server will produce a confidence score for each image.

  ** Pre-partitioned vs stand-alone

  This is a partitioned version of *exec.py* in this folder. You can compare the running time with it.

 `./exec.py infer /root/openface/models/openface/celeb-classifier.nn4.small2.v1.pkl classify-test/raw/`

 This will predict images locally, with output on execution time.

 