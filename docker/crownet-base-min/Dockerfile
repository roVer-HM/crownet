# crownet base container
#

FROM ubuntu:24.04

ARG UBUNTU_MIRROR=https://mirror.netcologne.de/ubuntu/

ARG CROWNET_COMMIT_HASH=unknown
ENV RELEASE=${CROWNET_COMMIT_HASH}

LABEL description="CrowNet base container"
LABEL maintainer="wischhof@ieee.org"

# bootstrap: update ca-certificates (before changing to the mirror)
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       ca-certificates

# setup using mirror for all packages (uses https)
RUN set -eux; \
    codename="$(. /etc/os-release && printf '%s' "$VERSION_CODENAME")"; \
    cat > /etc/apt/sources.list.d/ubuntu.sources <<EOF
Types: deb
URIs: ${UBUNTU_MIRROR}
Suites: ${codename} ${codename}-updates ${codename}-backports ${codename}-security
Components: main restricted universe multiverse
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
EOF

# install all packages required
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
       sudo \
       openssh-client \
       git git-lfs \
       python3.12 python3 python3-pip \
       python3.12-venv python3-venv python3.12-tk python3-tk \
       curl wget \
       vim \
       openjdk-11-jre \
       linux-tools-common linux-tools-generic linux-cloud-tools-generic && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# download public key for sam-dev.cs.hm.edu
RUN mkdir -p -m 0600 ~/.ssh && ssh-keyscan -p 6000 sam-dev.cs.hm.edu >> ~/.ssh/known_hosts
