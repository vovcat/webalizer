#
# Makefile for webalizer - a web server logfile analysis thingie
#
# (c)1997, 1998 by Bradford L. Barrett (brad@mrunix.net)
# Distributed under the GNU GPL. See "README" and "Copyright"
# files supplied with this distribution for more information.
#
# Change CC, CFLAGS and GDLIB for your site.
# Default is to use GNU gcc type stuff, and
# GDLIB defaults to /usr/local/include/gd, so
# if you put it somewhere else, change to match.

# Use these for SCO (and maybe others)
#CC=cc
#CFLAGS= -b elf

# These for GNU gcc
CC=gcc
CFLAGS= -O2 -Wall -fsigned-char

GDLIB = /usr/local/include/gd
LIBS= -lgd -lm

all: webalizer

webalizer: webalizer.o webalizer.h graphs.o graphs.h ctry.h
	$(CC) ${CFLAGS} -o webalizer webalizer.o graphs.o ${LIBS}

webalizer.o: webalizer.c webalizer.h webalizer_lang.h ctry.h
	$(CC) $(CFLAGS) -c webalizer.c

graphs.o: graphs.c graphs.h webalizer_lang.h
	$(CC) $(CFLAGS) -I$(GDLIB) -c graphs.c

clean:
	rm -f *.o webalizer
	rm -f  usage*.gif daily*.gif hourly*.gif ctry*.gif
	rm -f *.html *.hist core

distclean: clean
	rm -f webalizer.conf *.gz *.tar *.tgz *.Z
	rm -f webalizer-static

install: webalizer
	cp webalizer ${DESTDIR}/usr/bin/webalizer
