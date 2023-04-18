# crownet base container
FROM arm64v8/ubuntu:20.04
# install all packages required
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    sudo \
    openssh-client \
    git git-lfs \
    python3.8 python3.7 python python3 python3-pip \
    python3.8-venv python3-venv python3.8-tk python3-tk \
    curl wget \
    vim \
    openjdk-11-jre \
    linux-tools-common linux-tools-generic && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# download public key for sam-dev.cs.hm.edu
RUN mkdir -p -m 0600 ~/.ssh && ssh-keyscan -p 6000 sam-dev.cs.hm.edu >> ~/.ssh/known_hosts
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    uncrustify nemiver \
    perl zlib1g-dev \
    clang bison flex make qt5-default  libqt5opengl5-dev \
    libstdc++6-8-dbg \
    clang-format \
    bear cmake \
    libboost-all-dev libcrypto++-dev libcrypto++-utils libcrypto++6 libgeographic-dev  \
    doxygen graphviz tcl-dev tk-dev  valgrind \
    openscenegraph libopenscenegraph-dev libosgearth-dev openscenegraph-plugin-osgearth \
    openscenegraph osgearth \
    libopenmpi-dev libxml2-dev libwebkit2gtk-4.0-37  && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --upgrade  numpy pandas matplotlib scipy seaborn posix_ipc

# download public key for sam-dev.cs.hm.edu
RUN mkdir -p -m 0600 ~/.ssh && ssh-keyscan -p 6000 sam-dev.cs.hm.edu >> ~/.ssh/known_hosts

# install google test library
RUN mkdir download
WORKDIR /download
RUN wget https://github.com/google/googletest/archive/release-1.12.0.tar.gz
RUN tar -xvf release-1.12.0.tar.gz
WORKDIR /download/googletest-release-1.12.0/
RUN cmake CMakeLists.txt
RUN make install
WORKDIR /
RUN rm -rf download

WORKDIR /opt
RUN git clone https://github.com/vlm/asn1c.git && \
    cd asn1c && git checkout 9925dbbda86b436896108439ea3e0a31280a6065
RUN cd asn1c && test -f configure || autoreconf -iv && \
    ./configure && \
    make install

# clone the OMNeT++ sources (version/tag is specified in the git command at the end)
WORKDIR /opt
# enable lfs (must be done before cloning!) and clone the project
RUN git lfs install


# add a symbolic link so omnetpp is found in the path

# pull again (so a cached layer having the clone gets updated)
WORKDIR /opt/
RUN --mount=type=ssh wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-6.0.1/omnetpp-6.0.1-linux-aarch64.tgz

RUN tar -xvzf omnetpp-6.0.1-linux-aarch64.tgz && \
    rm omnetpp-6.0.1-linux-aarch64.tgz && mv omnetpp-6.0.1 /opt/omnetpp

WORKDIR /opt/omnetpp/
RUN ln -s /opt/omnetpp /opt/omnetpp/omnetpp

# configure and compile
ENV PATH /opt/omnetpp/bin:$PATH
COPY configure.user .

RUN apt-get update -y
RUN export SWT_GTK3=1
RUN apt-get install -y libwebkit2gtk-4.0-37

WORKDIR /opt/omnetpp
SHELL ["/bin/bash", "-c"]

RUN apt update \
    && apt install -y firefox \
    openssh-server \
    xauth \
    && mkdir /var/run/sshd \
    && mkdir /root/.ssh \
    && chmod 700 /root/.ssh \
    && ssh-keygen -A \
    && sed -i "s/^.*PasswordAuthentication.*$/PasswordAuthentication no/" /etc/ssh/sshd_config \
    && sed -i "s/^.*X11Forwarding.*$/X11Forwarding yes/" /etc/ssh/sshd_config \
    && sed -i "s/^.*X11UseLocalhost.*$/X11UseLocalhost no/" /etc/ssh/sshd_config \
    && grep "^X11UseLocalhost" /etc/ssh/sshd_config || echo "X11UseLocalhost no" >> /etc/ssh/sshd_config
RUN apt install xdg-utils -y
RUN mkdir /usr/share/desktop-directories/
RUN  source setenv  && ./configure 
#WITH_QTENV=no WITH_OSG=no
RUN make && \
    find . -type f -name '*.o' -exec rm {} + && \
    find . -type f -name '*.o.d' -exec rm {} +



# update permissions so that the omnetpp directory becomes writable
RUN chmod -R a+rw /opt/omnetpp
# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
#ENTRYPOINT ["tail", "-f", "/dev/null"]
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh

