#########################################################################
#	Makefile for the Base simulation environment.			#
#	Use MPI=1 if you want to compile with MPI support.		#
#########################################################################

EXES	= Simulate
LIBNAME = lofarsim

#LIBS    = $(DIRPREFIX)../BaseSim/$(ALIBDIR)/libbasesim.a -L/usr/local/inprise/vbroker/lib -lorb_r -lpthread

LIBS    += $(DIRPREFIX)../BaseSim/$(ALIBDIR)/libbasesim.a

ifdef CORBA
LIBS += $(DIRPREFIX)../BaseSim/$(ALIBDIR)/libth_corba.a 
LIBS    += -L/usr/local/inprise/vbroker/lib -lvport_r -lorb_r -lpthread
endif

AIPSPP  = 1

include ../BaseSim/Makedefs
