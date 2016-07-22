all:
	i686-pc-mingw32-gcc main.c -shared -fPIC -o pp.dll

test: crc32.c
	gcc -std=c99 crc32.c -o test
	./test.exe

clean:
	rm -rf test.exe
