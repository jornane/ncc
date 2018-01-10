prefix=/usr/local

ncc:

install: ncc
	install -d -m 0755 $(prefix)/bin
	install -Cv -m 0755 ncc $(prefix)/bin/ncc
