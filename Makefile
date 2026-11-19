CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I src
LDFLAGS = -lm

SRCS = src/array.c src/geom.c src/parser.c src/ir.c src/toolchain.c src/main.c
OBJS = $(SRCS:.c=.o)

all: hao

hao: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) hao

.PHONY: all clean
