FROM ubuntu:xenial

RUN apt-get update && \
    apt-get install -y --no-install-recommends software-properties-common && \
    add-apt-repository ppa:team-gcc-arm-embedded/ppa -y && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    bzip2 \
    dfu-util \
    gcc-arm-embedded \
    git \
    libreadline-dev \
    unzip \
    wget && \
    rm -rf /var/lib/apt/lists/*

RUN wget --quiet https://www.lua.org/ftp/lua-5.3.4.tar.gz -O lua.tar.gz && \
    wget --quiet https://luarocks.org/releases/luarocks-3.0.4.tar.gz -O luarocks.tar.gz
RUN tar -xzf lua.tar.gz && \
    cd lua-5.3.4 && \
    make linux test && \
    make install && \
    cd .. && \
    tar -xzpf luarocks.tar.gz && \
    cd luarocks-3.0.4 && \
    ./configure && make bootstrap && \
    cd ..
RUN luarocks install fennel

WORKDIR /target
ENTRYPOINT ["make"]
