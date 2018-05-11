
CC = gcc
CFLAGS = -g -O2 -Wall -Wundef
LDLIBS = -lm
OBJECTS =

all: search

seach: search.c
	$(CC) $(CFLAGS) $^ -o search $(LDLIBS)

clean:
	rm -rf search
	git checkout -- *.gv *.ps *.png

