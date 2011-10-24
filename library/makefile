
CFLAGS_BASE=-Wall -I.. -I../aipc -I/site/include -I/usr/local/include -g -fPIC -D_REENTRANT -O3
CFLAGS=$(CFLAGS_BASE) -Werror
CFLAGS_NOWERROR=$(CFLAGS_BASE)

OBJS=jack.o dummies.o

all: libk_jack.a

clean:
	rm *.o libk_jack.a *~

libk_jack.a: $(OBJS)
	ar rus libk_jack.a $(OBJS)

.c: .o
	gcc $(CFLAGS) $*.c
