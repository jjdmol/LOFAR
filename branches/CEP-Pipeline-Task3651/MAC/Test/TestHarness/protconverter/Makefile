CFLAGS  += -Wall -ggdb -I../../../../installed/gnu_debug/include
CC = g++

default: converter

# To create the executable file count we need the source files
#

converter:   protconvert.o
	$(CC) $(CFLAGS) -o converter protconvert.c
	@ echo "It's done!"


protconvert.o:   protconvert.c
	$(CC) $(CFLAGS) -c protconvert.c

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
all: converter

clean:
	rm -f converter *.o *~ core *.tmp

