TARGET     = bb

MPI_HOME   = /opt/scali
MPI_LDLIBS = -lpthread -lmpi -L$(MPI_HOME)/contrib/mpe/lib -lmpe -llmpe -L$(MPI_HOME)/lib 
MPI_CXX_LDLIBS = -L/opt/scali/contrib/mpich/lib/ -lpmpich++
MPI_INCLUDES = -I$(MPI_HOME)/include
MPI_CXX_INCLUDES = -I$(MPI_HOME)/contrib/mpich/include

DTLINCLUDE = ${HOME}/include/dtl
DTLLIBDIR  = ${HOME}/lib
DTLLIB     = -lDTL

SOURCES = \
		blackboard/MPIProgramEntry.cpp \
		blackboard/BlackBoard.cpp \
		blackboard/main.cpp \
		control/Control.cpp \
		control/TopLevelStrategy.cpp \
		knowledge/WorkerWrapper.cpp \
		directives/Directive.cpp\
		directives/DirectiveData.cpp\
		directives/Parser.cpp\

#		directives/SelectScriptID.cpp\


OBJS       = $(SOURCES:.cpp=.o)

CCC     = g++
#OPT     = -O3
OPT     = -pg -g

CFLAGS     = -Wall -D_REENTRANT -I./ -I$(DTLINCLUDE) $(MPI_INCLUDES) $(MPI_CXX_INCLUDES) $(OPT) $(CC_MACH_FLAGS) 

%.o : %.cpp Makefile
	$(CCC) -c $(CFLAGS) $< -o $@

$(TARGET):   $(OBJS)
	$(CCC)  $(MPI_CXX_LDLIBS) $(OBJS) -o $(TARGET) \
	      $(MPI_LDLIBS) $(MPI_CXX_LDLIBS) $(MACH_LDFLAGS) -L$(DTLLIBDIR) $(DTLLIB) -lodbc

bbdb:	blackboard/createdb.o
	$(CCC) $< -o $@ \
	      -L$(MPI_HOME)/lib $(MPI_LDLIBS) $(MACH_LDFLAGS) -L$(DTLLIBDIR) $(DTLLIB) -lodbc

clean:
	rm -f $(OBJS) $(TARGET).o $(TARGET) core 

new:
	make clean
	make
