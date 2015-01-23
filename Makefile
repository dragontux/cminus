CC=clang
CFLAGS=-g -Wall -Iinclude

.PHONY: all
all: tags cminus

SRC=$(wildcard src/*.c)
OBJS=$(SRC:.c=.o)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

tags : $(SRC)

cminus: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

.PHONY: clean
clean:
	-rm -f src/*.o cminus tags
