
OS = Lnx86

INSTALL   = install -c -s -m 0755
BINDIR    = ../$(OS)/bin
LIBDIR    = ../$(OS)/lib
INCDIR    = ../include
EXPORTDIR = /usr/local/Spoutnik
DATE          = `date +%Y%m%d_%H%M%S`
EXPORTSRCSDIR = $(EXPORTDIR)/src
EXPORTBCKSDIR = $(EXPORTDIR)/src_$(DATE)
EXPORTSRCIDIR = $(EXPORTDIR)/include
EXPORTBCKIDIR = $(EXPORTDIR)/include_$(DATE)
EXPORTSRCBDIR = $(EXPORTDIR)/$(OS)
EXPORTBCKBDIR = $(EXPORTDIR)/$(OS)_$(DATE)

CC        = gcc
LD        = gcc

CFLAGS0  = -I$(INCDIR) -Wall
LDFLAGS0 = -L$(LIBDIR) -lm -Wall

### ------------------------------------------------------------
### OPTIMIZATION
### ------------------------------------------------------------
CCOptFLAGS = -O
LDOptFLAGS = -O
### ------------------------------------------------------------

### ------------------------------------------------------------
### DEBUG
### ------------------------------------------------------------
CCDbgFLAGS = -g -DDEBUG
LDDbgFLAGS = -g -DDEBUG
### ------------------------------------------------------------

### ------------------------------------------------------------
### GCOV
### ------------------------------------------------------------
CCGcovFLAGS = -fprofile-arcs -ftest-coverage
LDGCovFLAGS = -fprofile-arcs -ftest-coverage
### ------------------------------------------------------------


CFLAGS  = $(CFLAGS0) $(CCGcovFLAGS)
LDFLAGS =  $(LDFLAGS0) $(CCGcovFLAGS)

CFLAGS  = $(CFLAGS0) $(CCDbgFLAGS)
LDFLAGS =  $(LDFLAGS0) $(CCDbgFLAGS)

CFLAGS  = $(CFLAGS0) $(CCOptFLAGS)
LDFLAGS =  $(LDFLAGS0) $(CCOptFLAGS)


APPLICS = som prsom somindex CAH CAHclass somlabel somproba indexfile test_som test_psom


### ##################################################
###
### LES TARGETS
###
### ##################################################
help:;
	-@echo " "
	-@echo "  *** Makefile of Spoutnik Library ***"
	-@echo " "
	-@echo "  options:"
	-@echo " "
	-@echo "  make all"
	-@echo "         Compiles all applications { $(APPLICS) }"
	-@echo "  make clean"
	-@echo "         Removes all binary files (*.o) and executables"
	-@echo "  make install"
	-@echo "         Compiles all and install in binary directory: $(BINDIR)/"
	-@echo "  make exports            //// do it as root ////"
	-@echo "         Compiles all, cleans all binary and backup but executables, installs"
	-@echo "         and exports directories to export directory: $(EXPORTDIR)/"
	-@echo "  make safe_exports       //// do it as root ////"
	-@echo "         Same as precedent but DO NOT overwrite export subdirectories."
	-@echo "  make help"

all : $(APPLICS)


clean : clean_others
	-\rm $(APPLICS) 2> /dev/null

clean_others :;
	-\rm *.o *.da *.bb *.bbg 2> /dev/null

clean_baks :;
	-\rm *~ ../include/*~ 2> /dev/null

install :	all install_only

install_only :;
	@-mkdir ../$(OS) $(BINDIR) 2> /dev/null
	-for i in ${APPLICS}; do (		\
		echo " $$i ...";		\
		$(INSTALL) $$i $(BINDIR)/$$i;	\
	); done

###	$(INSTALL) dfkhac $(BINDIR)/

exports :	all clean_others clean_baks install_only
	@-cd ../; tar cf - src include $(OS) | (cd $(EXPORTDIR)/; tar xvf -)

safe_exports :	all clean_others clean_baks install_only
	-mv $(EXPORTSRCSDIR) $(EXPORTBCKSDIR)
	-mv $(EXPORTSRCIDIR) $(EXPORTBCKIDIR)
	mv $(EXPORTSRCBDIR) $(EXPORTBCKBDIR)
	@-cd ../; tar cf - src include $(OS) | (cd $(EXPORTDIR)/; tar xvf -)


### ##################################################
###
### LES PROGRAMMES
###
### ##################################################

som :		som.o outils.o spoutnik.o
	$(LD) -o $@ $@.o outils.o spoutnik.o  $(LDFLAGS)

prsom :		prsom.o outils.o spoutnik.o
	$(LD) -o $@ $@.o outils.o spoutnik.o  $(LDFLAGS)

somindex :	somindex.o outils.o spoutnik.o
	$(LD) -o $@ $@.o outils.o spoutnik.o  $(LDFLAGS)

CAH :		CAH.o CAHfunc.o outils.o spoutnik.o
	$(LD) -o $@ $@.o CAHfunc.o outils.o spoutnik.o  $(LDFLAGS)

CAHclass :	CAHclass.o CAHclass_func.o outils.o spoutnik.o
	$(LD) -o $@ $@.o CAHclass_func.o outils.o spoutnik.o  $(LDFLAGS)

somlabel :	somlabel.o outils.o spoutnik.o
	$(LD) -o $@ $@.o outils.o spoutnik.o  $(LDFLAGS)

somproba :	somproba.o outils.o spoutnik.o
	$(LD) -o $@ $@.o outils.o spoutnik.o  $(LDFLAGS)

indexfile :	indexfile.o outils.o spoutnik.o
	$(LD) -o $@ $@.o outils.o spoutnik.o  $(LDFLAGS)

test_som :	test_som.o outils.o spoutnik.o
	$(LD) -o $@ $@.o outils.o spoutnik.o  $(LDFLAGS)

test_psom :	test_psom.o outils.o spoutnik.o
	$(LD) -o $@ $@.o outils.o spoutnik.o  $(LDFLAGS)


### ##################################################
###
### LES MODULES
###
### ##################################################


som.o :		som.c      	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h $(INCDIR)/som.h

prsom.o :	prsom.c      	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h $(INCDIR)/prsom.h

somindex.o :	somindex.c 	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h $(INCDIR)/somindex.h

CAH.o :		CAH.c      	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h $(INCDIR)/CAH.h

CAHclass.o :	CAHclass.c     	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h $(INCDIR)/CAHclass.h

somlabel.o :	somlabel.c	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h $(INCDIR)/somlabel.h

somproba.o :	somproba.c    	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h $(INCDIR)/somproba.h

indexfile.o :	indexfile.c    	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h $(INCDIR)/prsom.h

test_som.o :	test_som.c      $(INCDIR)/outils.h $(INCDIR)/spoutnik.h

test_psom.o :	test_psom.c     $(INCDIR)/outils.h $(INCDIR)/spoutnik.h


spoutnik.o :	spoutnik.c 	$(INCDIR)/outils.h $(INCDIR)/spoutnik.h

outils.o :	outils.c 	$(INCDIR)/outils.h
