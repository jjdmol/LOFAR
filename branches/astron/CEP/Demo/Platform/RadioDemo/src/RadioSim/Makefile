#########################################################################
#	Makefile for the Base simulation environment.			#
#	Use MPI=1 if you want to compile with MPI support.		#
#########################################################################

DIRPREFIX= ../
EXES	= Simulate
LIBNAME = lofarsim
LIBS    = $(DIRPREFIX)../BaseSim/$(ALIBDIR)/libbasesim.a

AIPSPP  = 1

include $(DIRPREFIX)../BaseSim/Makedefs 

ifdef CORBA
LIBS += $(DIRPREFIX)../BaseSim/$(ALIBDIR)/libth_corba.a 
endif
