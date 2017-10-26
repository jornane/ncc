# Introduction

Netcat clone.

# Prepare

Install [Meson](http://mesonbuild.com/).

## Debian/Ubuntu

	$ sudo apt-get install meson build-essential

## CentOS/RHEL

	$ sudo yum install meson gcc

## Fedora

	$ sudo dnf install meson gcc
# Compile

	$ meson build
	$ cd build
	$ ninja

# Run
	
	$ ./ncc <host> <port>
