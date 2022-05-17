CC?=clang

SRC=src/*
GAME=game

LIBS!=sdl2-config --cflags --libs
INCLUDE=-Iinclude

$(GAME): $(SRC)
	mkdir -p bin
	$(CC) $(LIBS) -o bin/$(GAME) $(INCLUDE) $(SRC)

clean: 
	rm -r bin/
