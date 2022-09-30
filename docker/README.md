
IOWA-docker
============

Multiple Dockerfile for [IOWA](https://ioterop.com/iowa)

All versions of the image are built from the same Github code (it means that certain features, specific to the commercial version, are not supported on some samples). 

---

## Changelog

* **01-Oct-2022** - Initial release
    - Base image for Ubuntu 20.04
    - Base image for Raspberry Pi, base on Raspbian Stretch

---

## Pre-Requisites

- install docker on your host [https://docs.docker.com/engine/install/](https://docs.docker.com/engine/install/)

## Usage

Build the iowa_sdk_lite image:
- ``` docker build --no-cache --tag iowa_sdk_lite .```

Run the *iowa_sdk_lite* image interactively:
- ```docker run -it iowa_sdk_lite```    

Run a unique sample (e.g.: IPSO_client):
- ```docker run --init iowa_sdk_lite ./bin/IPSO_client```  


## Note

- Building image for Raspberry pi is quite long. The reason is that the *raspbian/strech* image gets a incompatible version of cmake (too old) and we need to rebuild it. 

- Running the docker image with *--init* allows to stop the sample using Ctrl-C . If you forgot this parameter, to stop the running sample,  you can open a new shell and execute: 
```
$ docker ps # get the id of the running container
$ docker stop <container> # kill it (gracefully)
