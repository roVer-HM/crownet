# Building the Docker Containers

## Docker Version
* Some Dockerfiles use experimental features and require Docker buildkit
* Docker 20.10 or later is required
* At the time of writing, we currently use Docker version 29.4.0, build 9d7ad9f.

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

## Tagging

Usually, we refer to individual containers using tags with corresponding version numbers. To add an additional version tag to the latest container you have built, use the docker `tag` command, e.g.

```
docker image tag sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp:latest sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp:6.3.0
```

