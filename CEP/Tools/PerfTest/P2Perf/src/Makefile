#########################################################################
#	Makefile for the Base simulation environment.			#
#	Use MPI=1 if you want to compile with MPI support.		#
#########################################################################

EXES	= Simulate
LIBNAME = lofarsim
LIBS    = $(DIRPREFIX)../BaseSim/$(ALIBDIR)/libbasesim.a -L/opt/inprise/vbroker/lib -lcosev_r -lcosnm_r -lvport_r -lorb_r -lpthread

ifdef CORBA
LIBS += $(DIRPREFIX)../BaseSim/$(ALIBDIR)/libth_corba.a 
endif

AIPSPP  = 1

include ../BaseSim/Makedefs
