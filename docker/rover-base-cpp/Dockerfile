# roVer base container
#

FROM sam-dev.cs.hm.edu:5023/rover/crownet/rover-base-min:latest

LABEL description "roVer base container"
LABEL maintainer "stefan.schuhbaeck@hm.edu"


# install all packages required to compile omnet
# Note: Since clang does not emit debug info for types within the standard libraries,
#       we must include debug info for the installed libraries (versions must match!)
#       by manually installing the libstdc++6-8-dbg package
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       uncrustify nemiver \
       clang bison flex make qt5-default \
       libstdc++6-8-dbg \
       clang-format \
       googletest libgtest-dev google-mock \
       bear cmake \
       libboost-all-dev libcrypto++-dev libcrypto++-utils libcrypto++6 libgeographic-dev  \
       doxygen graphviz tcl-dev tk-dev  valgrind \
       libopenscenegraph-3.4 libopenscenegraph-3.4-dev libosgearth-dev openscenegraph-plugin-osgearth \
       openscenegraph osgearth \
       libopenmpi-dev libxml2-dev libwebkitgtk-3.0-0  && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN cd /usr/src/googletest/googletest && cmake CMakeLists.txt && make install
RUN cd /usr/src/googletest/googlemock && cmake CMakeLists.txt && make install
