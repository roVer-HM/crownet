# Typical Setup of a Simulation Machine

## Base Distribution

* Ubuntu 18.04.4 LTS
* General dependencies:

```
sudo apt install git git-lfs htop
```

## OMNeT++ Simulation Environment
   * OMNeT++ Version 5.4.1
   * Depends on:

```
   apt install build-essential clang lld bison flex perl tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf automake libtool libgdal-dev libfox-1.6-dev libxerces-c-dev qt5-default openscenegraph-3.4 openscenegraph-plugin-osgearth libosgearth-dev libopenscenegraph-3.4-dev openscenegraph-3.4-doc
```
   ### OMNeT++ Installation
   ``` 
   git clone https://sam-dev.cs.hm.edu/rover/omnetpp.git
   cd omnetpp
   git checkout hm_master
   mkdir bin
   ./configure
   make -j8
   ``` 