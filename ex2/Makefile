
CC = gcc
CFLAGS = -g -O2 -Wall -Wundef
LDLIBS = -lm
OBJECTS =

all: search

search: search.c
	$(CC) $(CFLAGS) $^ -o search $(LDLIBS)

.PHONY: clean
clean:
	rm -rf search
	git checkout -- *.gv *.ps *.png

