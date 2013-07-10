
CFLAGS = -g

all: puzzle generate

puzzle: puzzle.c
	gcc $(CFLAGS) -o puzzle puzzle.c

generate: generate.c
	gcc $(CFLAGS) -o generate generate.c -lm

clean:
	
spotless: clean
	-rm puzzle generate

