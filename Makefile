# HERMETIC-Ω (hermetic-array) · Makefile
# SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
# License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
# Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
# Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
# Hark, though this node be but a spark,
# Its covenant endureth through the dark.

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
