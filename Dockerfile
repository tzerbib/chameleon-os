FROM debian:latest
# FROM archlinux:latest

RUN apt-get update && apt-get -y upgrade
RUN apt-get -y install build-essential gcc-multilib qemu-system-x86 netcat-openbsd
RUN apt-get -y install iproute2 iputils-ping
RUN apt-get -y install binutils-i686-gnu gcc-i686-linux-gnu
RUN apt-get -y install gdb

# RUN yes | pacman -Syu
# RUN yes | pacman -S base-devel gdb git qemu-system-x86 iproute2

# RUN git clone https://aur.archlinux.org/i686-elf-gcc.git
# RUN pushd i686-elf-gcc
# RUN makepkg -i
# RUN popd

# RUN git clone https://aur.archlinux.org/i686-elf-binutils.git
# RUN pushd i686-elf-gcc
# RUN makepkg -i
# RUN popd

RUN mkdir -p /src/xv6-net
WORKDIR /src/xv6-net
COPY . .

RUN make xv6.img fs.img

RUN echo 'add-auto-load-safe-path /' > /root/.gdbinit
