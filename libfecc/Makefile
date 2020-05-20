# __MAKEFILE__

CC=gcc # C language compiler
COMMONFLAGS=-Wall -std=gnu99
CFLAGS =-ggdb3

HEADERFILES=$(shell find . -type f -name "*.h")
SOURCEFILES=$(shell find . -type f -name "*.c")

OBJFILES=${SOURCEFILES:c=o}
EXECUTABLE=main

all: build-file

$(OBJFILES): %.o: %.c $(HEADERFILES)
	$(CC) -c $< -o $@ $(COMMONFLAGS) $(CFLAGS)

build-file: $(OBJFILES)
	$(CC) -o $(EXECUTABLE) $(OBJFILES) $(LINKFLAGS)
