# __MAKEFILE__

CC=gcc # C language compiler
ifeq ($(RELEASE), 1)
	CFLAGS =-O3 -Wall -std=c99 -I/usr/include/mysql -I/usr/include/mysql/mysql
else
	CFLAGS =-ggdb3 -Wall -std=c99 -I/usr/include/mysql -I/usr/include/mysql/mysql
endif
LINKFLAGS=-lmysqlclient

HEADERFILES= \
	./general/defines.h \
	./general/strings.h \
	./general/binarysearch.h \
	./data-structures/hash_table.h \
	./data-structures/list.h \
	./files/file.h \
	./files/filemgmt.h \
	./memdbg/memory.h \
	./databases/database.h \
	./config/config.h \
	./protocol/protocol.h \
	./networking/network.h \
	./parser/parser.h \
	./parser/lexer.h \
	./logger/logs.h \
	./threading/threads.h \
	./errors/errorhandler.h \
	./data-structures/linked_list.h

SOURCEFILES= \
	./general/binarysearch.c \
	./general/strings.c \
	./data-structures/hash_table.c \
	./data-structures/list.c \
	./files/filemgmt.c \
	./files/file.c \
	./memdbg/memory.c \
	./main.c \
	./databases/database.c \
	./config/config.c \
	./protocol/create.c \
	./protocol/notification.c \
	./protocol/protocol.c \
	./networking/network.c \
	./parser/parser.c \
	./parser/lexer.c \
	./logger/logs.c \
	./threading/threads.c \
	./errors/errorhandler.c \
	./data-structures/linked_list.c \
	./protocol/response.c \
	./protocol/download.c \
	./protocol/constants.c \
	./protocol/request.c \
	./protocol/update.c


OBJFILES=${SOURCEFILES:c=o}
EXECUTABLE=main

all: build-file clean-logs

$(OBJFILES): %.o: %.c $(HEADERFILES)
	@$(CC) -c $< $(CFLAGS) -o $@

build-file: $(OBJFILES)
	$(CC) -o $(EXECUTABLE) $(OBJFILES) $(LINKFLAGS)

clean: 
	git clean -xid

clean-logs:
	@cd ./logs/ && rm * -I || echo "No logs to clear";
	@cd ./temp/ && rm * -I || echo "No temp files to clear";