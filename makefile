CC?=clang

SRC=src/*
GAME=game

SDL_LIBS!=sdl2-config --cflags --libs
LIBS=-lm $(SDL_LIBS) -lSDL2_ttf
CFLAGS?=-std=c17 -Wall -pedantic -Werror
INCLUDE=-Iinclude

$(GAME): $(SRC)
	mkdir -p bin
	$(CC) $(SRC) $(LIBS) -o bin/$(GAME) $(INCLUDE) $(CFLAGS)

compile_flags.txt:
	echo "$(LIBS) $(INCLUDE)" | sed 's/ /\n/g' > compile_flags.txt

clean: 
	rm -r bin/
