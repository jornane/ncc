CC=cc
CCFLAGS=-Wall

ncc: ncc.c
	$(CC) $(CCFLAGS) ncc.c -o ncc
