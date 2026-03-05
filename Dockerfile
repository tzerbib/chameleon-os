FROM debian:latest

RUN apt-get update && apt-get -y upgrade
RUN apt-get -y install build-essential gcc-multilib qemu-system-x86 netcat-openbsd
RUN apt-get -y install iproute2 iputils-ping

RUN mkdir -p /src/xv6-net
WORKDIR /src/xv6-net
COPY . .
RUN make xv6.img fs.img
