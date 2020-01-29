# __MAKEFILE__

CC=gcc # C language compiler
CFLAGS=-ggdb3 -Wall -std=c99
HEADERFILES=networking/defines.h networking/network.h list.h \
				memory.h hash_table.h dbp.h logger.h strings.h binarysearch.h
SOURCEFILES=networking/network.c server.c list.c memory.c \
				hash_table.c dbp.c logger.c strings.c binarysearch.c
OBJFILES=${SOURCEFILES:c=o}

program: $(SOURCEFILES) $(HEADERFILES)
	$(CC) -c $^ $(CFLAGS)
	$(CC) -o $@ *.o $(CFLAGS)

program-speed: $(SOURCEFILES) $(HEADERFILES)
	$(CC) -o $@ $(SOURCEFILES) -std=c99 -O3

clean: 
	git clean -xi

clean-logs:
	rm ./logs/* -ri