# __MAKEFILE__

CC=gcc # C language compiler
ifeq ($(RELEASE), 1)
    CFLAGS =-O3 -Wall -std=c99 -I/usr/include/mysql -I/usr/include/mysql/mysql
else
	CFLAGS =-ggdb3 -Wall -std=c99 -I/usr/include/mysql -I/usr/include/mysql/mysql
endif
LINKFLAGS=-lmysqlclient

HEADERFILES=defines.h networking/network.h list.h \
				memory.h hash_table.h dbp.h logs.h strings.h binarysearch.h \
				./protocol/protocol.h file.h ./databases/database.h \
				config.h ./filemgmt.h ./parser/parser.h ./parser/lexer.h
SOURCEFILES=networking/network.c server.c list.c memory.c \
				hash_table.c dbp.c logs.c strings.c binarysearch.c \
				./protocol/notification.c ./protocol/create.c ./file.c \
				./protocol/protocol.c ./databases/database.c \
				config.c ./filemgmt.c ./parser/lexer.c ./parser/parser.c

OBJFILES=${SOURCEFILES:c=o}
EXECUTABLE=main
OBJDIR=./objects/

all: link-program clean-logs

program: clean-logs
$(OBJFILES): %.o: %.c
	@$(CC) -c $< $(CFLAGS) -o $(OBJDIR)`basename $@`

link-program: mkobjdir $(OBJFILES)
	$(CC) -o $(EXECUTABLE) $(OBJDIR)*.o $(LINKFLAGS)

mkobjdir: 
	@mkdir -p ./objects/

clean: 
	git clean -xid

clean-logs:
	@cd ./logs/ && rm * -I || echo "No logs to clear";