#CFLAGS=-O3 -mfpmath=sse,387 -march=nocona

CFLAGS=-O3

all: md5a32 a32

md5a32: md5a32.o adler32.o

md5a32.o: md5.h

adler32: adler32.c adler32.h
	gcc -c adler32.c -o adler32.o

a32: a32.o adler32.o

clean:
	rm -f adler32.o md5a32.o a32.o md5a32 a32

