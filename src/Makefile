CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic

BINARIES=parser.o scanner.o stack.o symtable.o dynamicstring.o codegen.o prectable.o

all: compiler


compiler: $(BINARIES)
	$(CC) $(CFLAGS) -o compiler $(BINARIES)


###############################################

parser.o: parser.c parser.h error.h scanner.h symtable.h
	$(CC) $(CFLAGS) -c parser.c

scanner.o: scanner.c scanner.h error.h
	$(CC) $(CFLAGS) -c scanner.c

stack.o: stack.c stack.h scanner.h
	$(CC) $(CFLAGS) -c stack.c

symtable.o: symtable.c symtable.h error.h
	$(CC) $(CFLAGS) -c symtable.c

dynamicstring.o: dynamicstring.c dynamicstring.h error.h
	$(CC) $(CFLAGS) -c dynamicstring.c

codegen.o: codegen.c codegen.h error.h dynamicstring.h
	$(CC) $(CFLAGS) -c codegen.c

prectable.o: prectable.c error.h scanner.h parser.h
	$(CC) $(CFLAGS) -c prectable.c

###############################################

clean:
	rm *.o compiler

test:
	make
	./test.sh 'tests/*py'
