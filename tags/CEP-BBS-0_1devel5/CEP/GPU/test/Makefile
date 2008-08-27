HPMAIN=HPCorrelator
HPSRCS = HPCorrelator.cc GPUEnvironment.cc Stream.cc PixelBuffer.cc
FTMAIN=FTtest1
FTSRCS = FTtest1.cc GPUEnvironment.cc Stream.cc PixelBuffer.cc


OS="Linux"
CC=g++
CFLAGS= -Wall -g -O3
INCLUDES= 
LFLAGS= -L/usr/X11R6/lib/

#CFLAGS += -DUNIX
LFLAGS += -lX11 -lXmu -lXt -lGL -lGLU -lCg -lCgGL -lpthread

OBJS = $(SRCS:.c=.o)
HPOBJS = $(HPSRCS:.c=.o)
FTOBJS = $(FTSRCS:.c=.o)

all:	$(HPMAIN) $(FTMAIN)
	
$(HPMAIN):  $(HPOBJS)
	$(CC) $(HPOBJS) -o $(HPMAIN) $(CFLAGS) $(LFLAGS)

$(FTMAIN):  $(FTOBJS)
	$(CC) $(FTOBJS) -o $(FTMAIN) $(CFLAGS) $(LFLAGS)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)
