GPULIB=libGPU.a
GPUSRCS = GPUEnvironment.cc Stream.cc PixelBuffer.cc
FIRMAIN=FIRtest
FIRSRCS = FirFilter.cc 
MIPMAPMAIN=MIPMAPtest
MIPMAPSRCS = FirMipmap.cc 


OS="Linux"
CC=g++
CFLAGS= -Wall -g -O3          # -pg
INCLUDES= 
LFLAGS= -L/usr/X11R6/lib/ -L.

#CFLAGS += -DUNIX
LFLAGS += -lX11 -lXmu -lXt -lGL -lGLU -lCg -lCgGL -lpthread -lGPU

OBJS = $(SRCS:.c=.o)
FIROBJS = $(FIRSRCS:.c=.o)
MIPMAPOBJS = $(MIPMAPSRCS:.c=.o)

all:	$(GPULIB) $(FIRMAIN) $(MIPMAPMAIN)
	
$(FIRMAIN):  $(FIROBJS) $(GPULIB)
	$(CC) $(FIROBJS) -o $(FIRMAIN) $(CFLAGS) $(LFLAGS)

$(MIPMAPMAIN):  $(MIPMAPOBJS) $(GPULIB)
	$(CC) $(MIPMAPOBJS) -o $(MIPMAPMAIN) $(CFLAGS) $(LFLAGS)

$(GPULIB):	$(GPUSRCS)
	$(CC) -c $?
	ar r $(GPULIB) $(?:.cc=.o)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)
