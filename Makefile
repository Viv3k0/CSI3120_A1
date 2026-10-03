# MiniScope front end in C -- CSI3120A Assignment 1
CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
OBJS   = miniscope.o lexer.o symtab.o parser.o

miniscope: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

miniscope.o: miniscope.c lexer.h parser.h symtab.h
lexer.o:     lexer.c lexer.h
symtab.o:    symtab.c symtab.h
parser.o:    parser.c parser.h lexer.h symtab.h

clean:
	rm -f $(OBJS) miniscope miniscope.exe
