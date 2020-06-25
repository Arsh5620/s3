# __MAKEFILE__

CC=gcc # C language compiler
# COMMONFLAGS=-Wall -std=gnu99
# CFLAGS =-pg -ggdb -Og
CFLAGS = -Ofast

HEADERFILES=$(shell find . -type f -name "*.h")
SOURCEFILES=$(shell find . -type f -name "*.c")

OBJFILES=${SOURCEFILES:c=o}
EXECUTABLE=main

all: run-unit-tests build-file

$(OBJFILES): %.o: %.c $(HEADERFILES)
	$(CC) -c $< -o $@  $(CFLAGS)

build-file: $(OBJFILES)
	gcc $(CFLAGS) -o $(EXECUTABLE) $(OBJFILES)

run-unit-tests:
	@echo "Starting unit tests before performing build..."
	cd ../unit-testing && make
	../unit-testing/main

clean:
	rm forwardecc.o finite-fields.o rs.o