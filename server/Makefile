# __MAKEFILE__

CC=gcc # C language compiler
COMMONFLAGS=-Wall -std=gnu99 -I/usr/include/mysql -I/usr/include/mysql/mysql -I.
DEBUGFLAG=-DDEBUG
PROFILEFLAG=-DPROFILE 
ifeq ($(RELEASE), 1)
	CFLAGS =-O3
else ifeq ($(PROFILE), 1)
	CFLAGS =-pg $(PROFILEFLAG)
else
	CFLAGS =-ggdb -O0 $(DEBUGFLAG)
endif
LINKFLAGS=-lmysqlclient -lssl -lcrypto -lsqlite3

HEADERFILES=$(shell find . -type f -name "*.h")
SOURCEFILES=$(shell find . -type f -name "*.c")

OBJFILES=${SOURCEFILES:c=o}
EXECUTABLE=main

all: build-file clean-logs update-revision-build

$(OBJFILES): %.o: %.c $(HEADERFILES)
	$(CC) -c $< -o $@ $(COMMONFLAGS) $(CFLAGS)

build-file: $(OBJFILES)
	$(CC) -pg -rdynamic  -o$(EXECUTABLE) $(OBJFILES) $(LINKFLAGS)

clean: 
	git clean -xid -e "backup*" -e "schema.db"

clean-logs:
	@cd ./logs/ && rm * -I || echo "No logs to clear";
	@cd ./temp/ && rm * -I || echo "No temp files to clear";

update-revision-build:
	@echo "updating build version number"
	@variable=`cat ./protocol/micro.inc` && printf "#define __VERSION_MICRO__ "$$(printf $$(echo $$variable | cut -f 3 -d ' ' | head -n 1)"+1\n" | tee ~/testfile | bc)"\n#define __VERSION_GITHASH__ \""$$(git rev-parse --short=10 HEAD)"\"" > ./protocol/micro.inc