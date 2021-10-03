sandbox: *.[ch]
	$(CC) -Wall -Wconversion -Wshadow -Wno-unused-function -I/usr/include/SDL2 main.c -DDEBUG=1 -O0 -g -o sandbox -lSDL2 -lm
release:
	$(CC) -Wall -Wconversion -Wshadow -Wno-unused-function -I/usr/include/SDL2 main.c -O3 -o sandbox -lSDL2 -lm
sandbox-linux.tar.gz: release
	tar --transform "s,^,FunctionSandbox/," -czf $@ sandbox sandboxes example.png

