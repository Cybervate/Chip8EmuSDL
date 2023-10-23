all:
	g++ -Isrc/include -Lsrc/lib -o main main.c -lmingw32 -lSDL2main -lSDL2