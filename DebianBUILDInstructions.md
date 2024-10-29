## Intro

This instructions are tested on WSL2 Ubuntu and WSL2 Debian. They should work on non-virtual Ubuntu and Debian without modifications.

## Instructions

1. Install required tools:
```console
$ sudo apt install -y gcc make autoconf pkg-config libext2fs-dev git
```
2. Clone repo and cd into local dir:
```console
$ git clone https://github.com/e2tools/e2tools.git
$ cd e2tools
```
3. Execute autoreconf:
```console
$ autoreconf -vis .
```
4. Before continuing create an installation directory, e.g. /home/ivan/e2toolsInstallDir and configure
```console
$ ./configure --prefix=/home/ivan/e2toolsInstallDir // has to be absolute path
```
5. Make and install
```console
$ make
$ make install
```
6. Remove debugging symbols to reduce size of files:
```console
$ make install-strip
```