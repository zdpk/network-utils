FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    git \
    pkg-config \
    arping \
    tcpdump \
    iputils-ping \
    net-tools \
    iproute2 \
    clangd \
    clang-format \
    clang-tidy \
    libpcap-dev \
    && rm -rf /var/lib/apt/lists/*

ENV PATH=$GOPATH/bin:$PATH

ENV USER=root
ENV HOME=/root

WORKDIR /workspace

SHELL ["/bin/bash", "-c"]

# raw socket permissions
RUN setcap cap_net_raw,cap_net_admin=eip /usr/bin/tcpdump

RUN ln -s /usr/bin/clang-14 /usr/bin/clang

# change user to root
USER root 