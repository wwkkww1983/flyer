all:
	mingw32-gcc-4.9.2.exe -std=c99 crc32.c -shared -o crc32.dll

test: crc32.c
	gcc -std=c99 crc32.c -o test
	./test.exe

clean:
	rm -rf test.exe
	rm -rf *.dll

