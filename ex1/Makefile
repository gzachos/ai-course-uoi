
CC = gcc
CFLAGS = -g -O2 -Wall -Wundef
LDLIBS = 
OBJECTS =

all: game game-non-optimal-min

game: game.c
	$(CC) $(CFLAGS) -DMIN_PLAYS_OPTIMALLY $^ -o game $(LDLIBS)

game-non-optimal-min: game.c
	$(CC) $(CFLAGS) -UMIN_PLAYS_OPTIMALLY $^ -o game-non-optimal-min $(LDLIBS)

clean:
	rm -rf game game-non-optimal-min

