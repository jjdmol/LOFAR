# Makefile
CC=gcc
CPP=g++
CFLAGS=
CPPFLAGS= -g  -std=gnu++11 
INCLUDE=-I../include/
LIBS=-lqpidmessaging -lqpidtypes -lpthread

# DEPS = .h
OBJ= QPIDqueueoverview.o brokermgt.o

all: test


%.o: %.cc $(DEPS)
	$(CPP) -c -o $@ $< $(INCLUDE) $(CPPFLAGS)

test: $(OBJ)
	$(CPP) -o $@ $^ $(CPPFLAGS) $(LIBS)

clean:
	rm -f *.o test
