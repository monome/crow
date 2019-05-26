FROM ubuntu:xenial
WORKDIR /home/dev

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
    build-essential \
    bzip2 \
    dfu-util \
    git \
    libreadline-dev \
    software-properties-common \
    unzip \
    wget
RUN apt clean
RUN add-apt-repository ppa:team-gcc-arm-embedded/ppa -y && \
    apt-get update && \
    apt-get install -y gcc-arm-embedded

# RUN wget --quiet https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2018q4/gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2 -O cortex_m.tar.bz2 && \
#     tar -xjf cortex_m.tar.bz2 && \
#     rm cortex_m.tar.bz2
# ENV PATH $PATH:/home/dev/gcc-arm-none-eabi-8-2018-q4-major/bin

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