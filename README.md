# Horrible `nc` clone

This piece of sh^Hoftware does roughly the same as [nc(1)](https://www.freebsd.org/cgi/man.cgi?query=nc&sektion=1)
which is found on most \*NIX systems.  I wrote this to learn C and I
have to say job well done on that, but other than that I won't make any
guarantees that it won't feed your cat to gremlins, it probably will.

ncc has been tested to compile and work on recent versions of FreeBSD,
NetBSD, OpenBSD and CentOS.  It will probably also work on other OSes,
YMMV.  It will only work as a TCP client, no UDP, no server mode and no
fancy features such as source address selection.

This version uses [select(2)](https://www.freebsd.org/cgi/man.cgi?query=select&sektion=2)
to do its job, I intend to also try other mechanisms.


## Compilation

Should be as easy as

	make

If your operating system doesn't come with a compiler you may rectify
the situation with:

	yum install gcc


## Installation

If you really like `ncc` and you'd like it to be available as a general
command, simply install it in **/usr/local/bin**.

	make install


## Usage

	ncc host port

The application knows no command line flags.
