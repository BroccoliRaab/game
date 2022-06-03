CC?=clang

SRC=src/*
GAME=game

SDL_LIBS!=sdl2-config --cflags --libs
LIBS=-lm $(SDL_LIBS)
CFLAGS?=-std=c17 -Wall -pedantic -Werror
INCLUDE=-Iinclude

$(GAME): $(SRC)
	mkdir -p bin
	$(CC) $(LIBS) -o bin/$(GAME) $(INCLUDE) $(SRC)

clean: 
	rm -r bin/
