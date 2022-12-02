#
# Makefile for HW6 : um
# 
CC = gcc

IFLAGS  = -I/comp/40/build/include -I/usr/sup/cii40/include/cii
CFLAGS  = -g -O2 -std=gnu99 -Wall -Wextra -Werror -pedantic $(IFLAGS)
LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64
LDLIBS  = -lcii40-O2 -lpnmrdr -lcii40-O2 -lm -lbitpack

EXECS   = test um writetests

all: $(EXECS)

test: testing.o registers.o segments.o bitpack.o commandParse.o operations.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

um: um.o segments.o bitpack.o commandParse.o operations.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	
writetests: umlabwrite.o umlab.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXECS)  *.o

