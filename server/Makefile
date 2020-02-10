# __MAKEFILE__

CC=gcc # C language compiler
CFLAGS=-ggdb3 -Wall -std=c99 -I/usr/include/mysql -I/usr/include/mysql/mysql
LINKFLAGS=-lmariadb
HEADERFILES=defines.h networking/network.h list.h \
				memory.h hash_table.h dbp.h logs.h strings.h binarysearch.h \
				./protocol/dbp_protocol.h file.h ./databases/database.h \
				config.h ./filemgmt.h
SOURCEFILES=networking/network.c server.c list.c memory.c \
				hash_table.c dbp.c logs.c strings.c binarysearch.c \
				./protocol/notification.c ./protocol/create.c ./file.c \
				./protocol/dbp_helpers.c ./databases/database.c \
				config.c ./filemgmt.c
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