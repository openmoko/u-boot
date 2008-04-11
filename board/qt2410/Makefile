#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a

OBJS	:= qt2410.o flash.o
SOBJS	:= lowlevel_init.o

.PHONY:	all

all:	$(LIB) lowevel_foo.bin

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

lowlevel_foo.o:	../neo1973/common/lowlevel_foo.S
	$(CC) -c -DTEXT_BASE=0x33F80000 -march=armv4 \
	  -o lowlevel_foo.o ../neo1973/common/lowlevel_foo.S

lowlevel_foo:	lowlevel_foo.o lowlevel_init.o \
		../neo1973/common/lowlevel_foo.lds
	$(LD) -T ../neo1973/common/lowlevel_foo.lds -Ttext 0x33f80000 -Bstatic \
          lowlevel_init.o lowlevel_foo.o -o lowlevel_foo

lowevel_foo.bin:	lowlevel_foo
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary \
	  lowlevel_foo lowlevel_foo.bin

clean:
	rm -f $(SOBJS) $(OBJS) lowlevel_foo lowlevel_foo.o

distclean:	clean
	rm -f $(LIB) core *.bak .depend lowlevel_foo.bin

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CPPFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

-include .depend

#########################################################################
