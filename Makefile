CC=clang
CFLAGS=-g -Wall

all: makestuff

OBJS=$(patsubst %.c,%.o,$(wildcard *c))

%.o : %.c
	$(CC) $(CFLAGS) -c $<

makestuff: $(OBJS)
	$(CC) $(CFLAGS) -o blarg $(OBJS)

clean:
	rm *.o

.PHONY: all
