#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3 sample implementation
# Author: Christopher Moretti
#--------------------------------------------------------------------
CC=gcc217

all: ft

clean:
	rm -f ft

clobber: clean
	rm -f ft_client.o *~

ft: ft.o ft_client.o dynarray.o path.o checkerFT.o nodeFT.o
	$(CC) ft.o ft_client.o dynarray.o path.o checkerFT.o nodeFT.o -o ft

nodeFT.o:  nodeFT.h dynarray.h checkerFT.h
	$(CC) -c nodeFT.c

ft_client.o: ft_client.c ft.h a4def.h
	$(CC) -c ft_client.c

ft.o: ft.h dynarray.h path.h checkerFT.h ft.c a4def.h
	$(CC) -c ft.c

dynarray.o: dynarray.c dynarray.h
	$(CC) -c dynarray.c

checkerFT.o: a4def.h path.h checkerFT.h checkerFT.c dynarray.h
	$(CC) -c checkerFT.c

path.o: path.h path.c dynarray.h a4def.h checkerFT.h
	$(CC) -c path.c