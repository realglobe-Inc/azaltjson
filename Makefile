# Makefile for azaltjson

NAME = azaltjson
DATE_TAG:=`date +%Y%m%d`

LIBJANSSON = jansson

CCOPT = $(PCCOPT) `pkg-config --cflags glib-2.0` -I./$(LIBJANSSON)/src -I$(AZProlog)/include
LDOPT = $(PLDOPT) -L./$(LIBJANSSON)/src/.libs -L$(AZProlog)/lib -lazp

#CFLAGS = -Wall -O2 $(CCOPT)
#CFLAGS = -O2 $(CCOPT)
CFLAGS = -g -O0 $(CCOPT) -Wall # -DDEBUG

LDLIBS = $(LDOPT) `pkg-config --libs glib-2.0` -ljansson

AZPCFLAGS = /message /s_verbos /cc $(CC) /ccopt "$(CCOPT)" /link_opt "$(LDOPT)"
AZPC = azpc
AZPC_SYSTEM_PL = \
	$(AZProlog)/system/pl/iso_pred.pl \
	$(AZProlog)/system/pl/fs_utility.pl \
	$(AZProlog)/system/pl/setof.pl \
	$(AZProlog)/system/pl/utility.pl

LIB2_SRC = #/lib crypto

FOR_DEBUG = /ccopt "-g -pg -O0" /link_opt "-g -pg -O0" /debug
FOR_TUNNING = # /h 256 /l 128 /g 128 /a 16 /s 128 /fast # /no

default: azaltjson.so

azaltjson.so: azaltjson.o plmodule.o
	$(CC) -shared -dynamiclib -o $@ $^ $(LDLIBS)

azaltjson.o: azaltjson.c
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

plmodule.c: plmodule.pl
	$(AZPC) -p $^ /ncc $(AZPCFLAGS)

plmodule.o: plmodule.c
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

clean:
	rm -f *~ .*~ *.o *.so mkaz userfile.c

install:
	cp -p azaltjson.so $(AZProlog)/lib/azprolog/ext/.

install_libjansson:
	cp -Rp $(LIBJANSSON)/src/.libs/libjansson.so* $(AZProlog)/lib/.

test:
	echo "?-halt." | prolog_c -c test.pl 2> /dev/null # | grep "^test "
