sandbox: *.[ch]
	$(CC) -Wall -Wconversion -Wshadow -Wno-unused-function -I/usr/include/SDL2 main.c -DDEBUG=1 -O0 -g -o sandbox -lSDL2 -lm
