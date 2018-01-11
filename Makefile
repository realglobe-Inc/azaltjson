# Makefile for azaltjson

NAME = azaltjson
DATE_TAG:=`date +%Y%m%d`

LIBJANSSON_INC_PREFIX = /usr
LIBJANSSON_LIB_PREFIX = /usr/local

CCOPT = $(PCCOPT) -I $(LIBJANSSON_INC_PREFIX)/include -I$(AZProlog)/include
LDOPT = $(PLDOPT) -L $(LIBJANSSON_LIB_PREFIX)/lib -L$(AZProlog)/lib -lazp

#CFLAGS = -Wall -O2 $(CCOPT)
#CFLAGS = -O2 $(CCOPT)
CFLAGS = -g -O0 $(CCOPT) -Wall # -DDEBUG

LDLIBS = $(LDOPT) -ljansson

AZPCFLAGS = /message /s_verbos /cc $(CC) /ccopt "$(CCOPT)" /link_opt "$(LDOPT)"
AZPC = azpc
AZPC_SYSTEM_PL = \
	$(AZProlog)/system/pl/iso_pred.pl \
	$(AZProlog)/system/pl/fs_utility.pl \
	$(AZProlog)/system/pl/setof.pl \
	$(AZProlog)/system/pl/utility.pl

FOR_DEBUG = /ccopt "-g -pg -O0" /link_opt "-g -pg -O0" /debug
FOR_TUNNING = # /h 256 /l 128 /g 128 /a 16 /s 128 /fast # /no

default: azaltjson.so

azaltjson.so: azaltjson.o azaltjson_plmodule.o
	$(CC) -shared -dynamiclib -o $@ $^ $(LDLIBS)

azaltjson.o: azaltjson.c
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

azaltjson_plmodule.c: azaltjson_plmodule.pl
	$(AZPC) -p $^ /ncc $(AZPCFLAGS)

azaltjson_plmodule.o: azaltjson_plmodule.c
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

clean:
	rm -f *~ .*~ *.o *.so mkaz userfile.c

install:
	cp -p azaltjson.so $(AZProlog)/lib/azprolog/ext/.

test:
	echo "?-halt." | prolog_c -c test.pl 2> /dev/null # | grep "^test "
