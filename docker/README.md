# Building the Docker Containers

## Docker Version
* Some Dockerfiles use experimental features and require Docker buildkit
* Docker 20.10 or later is required
* At the time of writing, we currently use Docker version 28.2.2, build e6534b4.

## Scripts for Building the Docker Images

To build the individual containers, CrowNet provides a simple script which is invoked with the name of the container to be build. For example, to build the omnetpp container, call

```
../scripts/build_and_push_image -i omnetpp
```

To rebuild all CrowNet images, an additional script can be called:

```
../scripts/build_all_images
```

By adding the option `-p` the created images are also pushed to the image registry.
