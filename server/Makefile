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
	./dbp.h \
	./config/config.h \
	./protocol/protocol.h \
	./networking/network.h \
	./parser/parser.h \
	./parser/lexer.h \
	./logs/logs.h \
	./threading/threads.h

SOURCEFILES= \
	./general/binarysearch.c \
	./general/strings.c \
	./data-structures/hash_table.c \
	./data-structures/list.c \
	./dbp.c \
	./files/filemgmt.c \
	./files/file.c \
	./memdbg/memory.c \
	./server.c \
	./databases/database.c \
	./config/config.c \
	./protocol/create.c \
	./protocol/notification.c \
	./protocol/protocol.c \
	./networking/network.c \
	./parser/parser.c \
	./parser/lexer.c \
	./logs/logs.c \
	./threading/threads.c \


OBJFILES=${SOURCEFILES:c=o}
EXECUTABLE=main
OBJDIR=./build/objects/

all: link-program clean-logs

program: clean-logs
$(OBJFILES): %.o: %.c
	@$(CC) -c $< $(CFLAGS) -o $(OBJDIR)`basename $@`

link-program: mkobjdir $(OBJFILES)
	$(CC) -o $(EXECUTABLE) $(OBJDIR)*.o $(LINKFLAGS)
	mv ./$(EXECUTABLE) ./build/$(EXECUTABLE)

mkobjdir: 
	@mkdir -p $(OBJDIR)

clean: 
	git clean -xid

clean-logs:
	@cd ./log-output/ && rm * -I || echo "No logs to clear";