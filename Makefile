# Generated automatically from Makefile.in by configure.
#
# Makefile for webalizer - a web server log analysis program
#
# Copyright (C) 1997-1999  Bradford L. Barrett (brad@mrunix.net)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version, and provided that the above
# copyright and permission notice is included with all distributed
# copies of this or derived software.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details (file "COPYING").
#

prefix  = /usr
exec_prefix = ${prefix}

BINDIR = ${exec_prefix}/bin
MANDIR = ${prefix}/man/man1
CC     = gcc
CFLAGS = -Wall -O2
LIBS   = -lgd -lm  -lpng -lz
DEFS   =  -DHAVE_GETOPT_H=1 -DHAVE_MATH_H=1
LDFLAGS=
INSTALL= /usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644

# where are the GD header files?
GDLIB  = /usr/include

# Shouldn't have to touch below here!

all: webalizer

webalizer: webalizer.o webalizer.h graphs.o graphs.h ctry.h webalizer_lang.h
	$(CC) ${LDFLAGS} -o webalizer webalizer.o graphs.o ${LIBS}

webalizer.o: webalizer.c webalizer.h webalizer_lang.h ctry.h
	$(CC) ${CFLAGS} ${DEFS} -c webalizer.c

graphs.o: graphs.c graphs.h webalizer_lang.h
	$(CC) ${CFLAGS} ${DEFS} -I${GDLIB} -c graphs.c

clean:
	rm -f webalizer *.o usage*.gif daily*.gif hourly*.gif ctry*.gif
	rm -f *.html *.hist *.current core

distclean: clean
	rm -f webalizer.conf *.tar *.tgz *.Z
	rm -f Makefile webalizer_lang.h config.cache config.log config.status
	ln -s lang/webalizer_lang.english webalizer_lang.h

install: all
	$(INSTALL_PROGRAM) webalizer ${DESTDIR}/${BINDIR}/webalizer
	#$(INSTALL_DATA) webalizer.1 ${DESTDIR}/${MANDIR}/webalizer.1
	#$(INSTALL_DATA) sample.conf ${DESTDIR}/etc/webalizer.conf.sample

uninstall:
	rm -f ${MANDIR}/webalizer.1
	rm -f ${BINDIR}/webalizer
	rm -f /etc/webalizer.conf.sample
	rm -f webalizer_lang.h
	ln -s lang/webalizer_lang.english webalizer_lang.h
