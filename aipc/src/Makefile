
CFLAGS_BASE=-Wall -I.. -I/site/include -I/usr/local/include -g -fPIC -D_REENTRANT -O3
CFLAGS=$(CFLAGS_BASE) -Werror
CFLAGS_NOWERROR=$(CFLAGS_BASE)
LDFLAGS=-lm

OBJS=audioplugin.o audioplugincaller.o input.o output.o lock.o pipe.o plugin.o plugincaller.o sharedmem.o variable.o dir.o receiver.o simpleio.o audiopluginmixer.o


all: libaipc.a test_host test_plugin test_audiohost test_audioplugin

clean:
	rm -f *.o libaipc.a *~ */*~ */*/*~ Makefile~ */\#* */*/\#
	rm -f test_host test_plugin a.out


test_host: tests/test_host.c libaipc.a
	gcc tests/test_host.c $(CFLAGS) -o test_host -L. -laipc -lpthread

test_plugin: tests/test_plugin.c libaipc.a
	gcc tests/test_plugin.c $(CFLAGS) -o test_plugin -L. -laipc -lpthread

test_audiohost: tests/test_audiohost.c libaipc.a
	gcc tests/test_audiohost.c $(CFLAGS) -o test_audiohost -L. -laipc -lpthread

test_audioplugin: tests/test_audioplugin.c libaipc.a
	gcc tests/test_audioplugin.c $(CFLAGS) -o test_audioplugin -L. -laipc -lpthread

libaipc.a: $(OBJS)
	ar rus libaipc.a $(OBJS)

.c: .o
	gcc $(CFLAGS) $*.c

sharedmem.o: sharedmem.c sharedmem_locking.c sharedmem_procinfo.c
	gcc $(CFLAGS) -c sharedmem.c






