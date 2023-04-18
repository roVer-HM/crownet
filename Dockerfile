FROM ghcr.io/omnetpp/omnetpp-base:u22.04-1 as base
LABEL maintainer="Rudolf Hornig <rudi@omnetpp.org>"

# first stage - build omnet
FROM base as builder
ARG VERSION=6.0.1
WORKDIR /root
RUN wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-$VERSION/omnetpp-$VERSION-core.tgz \
         --referer=https://omnetpp.org/ -O omnetpp-core.tgz --progress=dot:giga && \
         tar xf omnetpp-core.tgz && rm omnetpp-core.tgz
RUN mv omnetpp-$VERSION omnetpp
WORKDIR /root/omnetpp
# remove unused files and build
RUN . ./setenv && \
    ./configure WITH_LIBXML=yes WITH_QTENV=no WITH_OSG=no WITH_OSGEARTH=no && \
    make -j $(nproc) MODE=release base && \
    rm -r doc out test samples misc config.log config.status

# second stage - copy only the final binaries (to get rid of the 'out' folder and reduce the image size)
FROM base
ARG VERSION=6.0.1
ENV OPP_VER=$VERSION
RUN mkdir -p /root/omnetpp
WORKDIR /root/omnetpp
COPY --from=builder /root/omnetpp/ .
RUN chmod 775 /root/ && \
    mkdir -p /root/models && \
    chmod 775 /root/models
WORKDIR /root/models
ENV HOME=/root/
RUN echo 'PS1="omnetpp-$OPP_VER:\w\$ "' >> /root/.bashrc && \
    echo '[ -f "$HOME/omnetpp/setenv" ] && source "$HOME/omnetpp/setenv" -f' >> /root/.bashrc && \
    touch /root/.hushlogin
CMD ["/bin/bash", "--init-file", "/root/.bashrc"]