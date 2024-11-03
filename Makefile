CFLAGS=-Wall -pedantic

.PHONY: all
all: test test_stdlib 

test: test.o myio.o
	gcc -o $@ $^

test_stdlib: test_stdlib.o
	gcc -o $@ $^

test_intermingled: test_intermingled.o myio.o
	gcc -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f test test_stdlib test.o myio.o test_stdlib.o test_intermingled.o
