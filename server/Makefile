# __MAKEFILE__

CC=gcc # C language compiler
CFLAGS=-ggdb3 -Wall -Werror -std=c99
HEADERFILES=networking/defines.h networking/network.h list.h memory.h hash_table.h
SOURCEFILES=networking/network.c server.c list.c memory.c hash_table.c
OBJFILES=${SOURCEFILES:c=o}

program: $(SOURCEFILES) $(HEADERFILES)
	$(CC) -c $^ $(CFLAGS)
	$(CC) -o $@ *.o $(CFLAGS)

clean: 
	rm *.o ./program