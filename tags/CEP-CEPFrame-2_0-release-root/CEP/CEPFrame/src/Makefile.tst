#########################################################################
#	Makefile for the Base simulation environment.			#
#	Use MPI=1 if you want to compile with MPI support.		#
#########################################################################

# Get all .cc files
ALLEXSRCS = $(wildcard *.cc)
TESTEXES  = $(ALLEXSRCS:%.cc=%)
DIRPREFIX += ../

include ../Makefile
