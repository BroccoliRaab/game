CC?=clang

SRC=src/*
GAME=game

SDL_LIBS!=sdl2-config --cflags --libs
LIBS=-lm $(SDL_LIBS) -lSDL2_ttf -lSDL2_image
CFLAGS+=-O0 -g -std=c17 -Wall -pedantic 
INCLUDE=-Iinclude

$(GAME): $(SRC)
	mkdir -p bin
	$(CC) $(SRC) $(LIBS) $(CFLAGS) -o bin/$(GAME) $(INCLUDE) 
compile_flags.txt:
	echo "$(LIBS) $(INCLUDE)" | sed 's/ /\n/g' > compile_flags.txt

clean: 
	rm -r bin/
