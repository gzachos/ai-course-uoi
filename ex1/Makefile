
CC = gcc
CFLAGS = -g -O2 -Wall -Wundef
LDLIBS = 
OBJECTS =

all: game

seach: game.c
	$(CC) $(CFLAGS) $^ -o game $(LDLIBS)

clean:
	rm -rf *.out game
	git checkout -- *.gv *.ps

