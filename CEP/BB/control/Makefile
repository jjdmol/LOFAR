TARGET     = bb

MPI_HOME   = /opt/scali
MPI_LDLIBS = -lpthread -lmpi
# -L/opt/scali/contrib/mpe/lib -lmpe -llmpe 

DTLINCLUDE = ${HOME}/include/dtl
DTLLIBDIR  = ${HOME}/lib
DTLLIB     = -lDTL

SOURCES    = blackboard/MPIProgramEntry.cpp blackboard/BlackBoard.cpp blackboard/main.cpp
OBJS       = $(SOURCES:.cpp=.o)

CCC     = g++
#OPT     = -O3
OPT     = -pg -g

CFLAGS     = -D_REENTRANT -I./ -I$(DTLINCLUDE) -I$(MPI_HOME)/include $(OPT) $(CC_MACH_FLAGS) 

%.o : %.cpp
	$(CCC) -c $(CFLAGS) $< -o $@

$(TARGET):   $(OBJS)
	$(CCC) $(OBJS) -o $(TARGET) \
	      -L$(MPI_HOME)/lib $(MPI_LDLIBS) $(MACH_LDFLAGS) -L$(DTLLIBDIR) $(DTLLIB) -lodbc

clean:
	rm -f $(TARGET).o $(TARGET) core 
