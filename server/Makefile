# __MAKEFILE__

CC=gcc # C language compiler
COMMONFLAGS=-Wall -std=gnu99 -I/usr/include/mysql -I/usr/include/mysql/mysql
DEBUGFLAG=-DDEBUG
PROFILEFLAG=-DPROFILE 
ifeq ($(RELEASE), 1)
	CFLAGS =-O3
else ifeq ($(PROFILE), 1)
	CFLAGS =-pg $(PROFILEFLAG)
else
	CFLAGS =-ggdb3 $(DEBUGFLAG)
endif
LINKFLAGS=-lmysqlclient -lssl -lcrypto

HEADERFILES=$(shell find . -type f -name "*.h")
SOURCEFILES=$(shell find . -type f -name "*.c")

OBJFILES=${SOURCEFILES:c=o}
EXECUTABLE=main

all: build-file clean-logs update-revision-build

$(OBJFILES): %.o: %.c $(HEADERFILES)
	$(CC) -c $< -o $@ $(COMMONFLAGS) $(CFLAGS)

build-file: $(OBJFILES)
	$(CC) -pg -o $(EXECUTABLE) $(OBJFILES) $(LINKFLAGS)

clean: 
	git clean -xid -e "backup*"

clean-logs:
	@cd ./logs/ && rm * -I || echo "No logs to clear";
	@cd ./temp/ && rm * -I || echo "No temp files to clear";

update-revision-build:
	awk -i inplace 'NR==1{print $$1 " " $$2 " " $$3 + 1}' ./protocol/micro