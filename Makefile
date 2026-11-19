CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I src -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lm

SRCS = src/array.c src/geom.c src/main.c src/parser.c
OBJS = $(SRCS:.c=.o)

all: hao

hao: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) hao

test: hao
	./hao

.PHONY: all clean test
