#//  Makefile: is to be Makefile.am
#//
#//  Copyright (C) 2002
#//  ASTRON (Netherlands Foundation for Research in Astronomy)
#//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#//
#//  This program is free software; you can redistribute it and/or modify
#//  it under the terms of the GNU General Public License as published by
#//  the Free Software Foundation; either version 2 of the License, or
#//  (at your option) any later version.
#//
#//  This program is distributed in the hope that it will be useful,
#//  but WITHOUT ANY WARRANTY; without even the implied warranty of
#//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#//  GNU General Public License for more details.
#//
#//  You should have received a copy of the GNU General Public License
#//  along with this program; if not, write to the Free Software
#//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#//
#//  $Id$

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

FLEX	= ${HOME}/bin/flex
BISON	= ${HOME}/bin/bison
CXX     = g++
CC      = gcc
#OPT     = -O3
COPT     = -pg -g

BISONFLAGS = --defines --debug --locations --graph --token-table --report=all
#BISONFLAGS = --defines --debug --locations --graph --token-table --report=all
FLEXFLAGS  = # --bison-bridge --bison-locations --trace --yylineno --nodefault #--debug --verbose
CFLAGS     = -Wall -D_REENTRANT -I./ -I$(DTLINCLUDE) $(MPI_INCLUDES) $(MPI_CXX_INCLUDES) $(COPT) $(CC_MACH_FLAGS) 

%.o : %.cpp Makefile
	$(CXX) -c $(CFLAGS) $< -o $@

%.o : %.cc Makefile
	$(CXX) -c $(CFLAGS) $< -o $@

%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET):   $(OBJS)
	$(CXX)  $(MPI_CXX_LDLIBS) $(OBJS) -o $(TARGET) \
	      $(MPI_LDLIBS) $(MPI_CXX_LDLIBS) $(MACH_LDFLAGS) -L$(DTLLIBDIR) $(DTLLIB) -lodbc

bbdb:	blackboard/createdb.o
	$(CXX) $< -o $@ \
	      -L$(MPI_HOME)/lib $(MPI_LDLIBS) $(MACH_LDFLAGS) -L$(DTLLIBDIR) $(DTLLIB) -lodbc

clean:
	-@rm $(OBJS)
	-@rm $(TARGET).o
	-@rm $(TARGET)
	-@rm core 
	-@rm selfparse.tab.*
	-@rm read_script
	-@rm lex.selfparse.*
	-@rm start.*.selfcal*
	-@rm selfparse.output
	-@rm selfparse.vcg
	-@rm confdata/*.out

new:
	make clean
	make

lex.%.c:	parser/%.l Makefile
	$(FLEX) -P$* $(FLEXFLAGS) $<

lex.%.cc:	%.l Makefile
	$(FLEX) -+ -P$* $(FLEXFLAGS) $<

%.tab.c:	parser/%.y Makefile
	$(BISON) -p$* $(BISONFLAGS) parser/$*.y

read_script:	parser/parse_util.o parser/main.o selfparse.tab.o lex.selfparse.o
	$(CXX) $+ -o $@ -lfl -ly

lex.selfparse.o:	lex.selfparse.c

.PRECIOUS:	selfparse.tab.c

%.d:	confdata/%.selfcal read_script
	./read_script $<