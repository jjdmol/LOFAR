# Written for GNU -*- makefile -*-.

CC = CC
CPPFLAGS += -I"$(ODBCHOME)/include"
CFLAGS += +w -mt -DSOLARIS -DANSI_C -DUNIX
LDFLAGS += -L"$(ODBCHOME)/lib" $(CFLAGS)
AR = $(CXX)
ARFLAGS = $(CXXFLAGS) -xar -o

include ../config/unixODBC.inc
MAKEFILEDEPS += ../config/unixODBC.inc

clean::
	-$(RM) -r SunWS_cache
