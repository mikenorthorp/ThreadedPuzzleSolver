
CFLAGS = -g -pthread

all: puzzle generate

puzzle: puzzle.c
	gcc $(CFLAGS) -o puzzle puzzle.c

generate: generate.c
	gcc $(CFLAGS) -o generate generate.c -lm

clean:
	-rm generate puzzle

spotless: clean
	-rm puzzle generate

