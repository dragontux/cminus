CC=clang
CFLAGS=-g -Wall

all: makestuff

OBJS=$(patsubst %.c,%.o,$(wildcard *c))

%.o : %.c
	$(CC) $(CFLAGS) -c $<

makestuff: $(OBJS)
	$(CC) $(CFLAGS) -o cminus $(OBJS)

clean:
	rm *.o cminus

.PHONY: all
