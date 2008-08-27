#########################################################################
#	Makefile for the Base simulation environment.			#
#	Use MPI=1 if you want to compile with MPI support.		#
#########################################################################

DIRPREFIX=../
EXES	= Simulate
LIBNAME = lofarsim
LIBS    = ../../BaseSim/$(ALIBDIR)/libbasesim.a

#AIPSPP  = 1

include ../../BaseSim/Makedefs 

ifdef SCACORBA
LIBS += -L/usr/local/inprise/vbroker/lib
else
LIBS += -L/usr/local/inprise/vbroker/lib
endif

ifdef CORBA
LIBS += -lorb_r -lpthread
LIBS += ../../BaseSim/$(ALIBDIR)/libth_corba.a 
endif
