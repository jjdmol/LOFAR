# Written for GNU -*- makefile -*-.

CC = aCC
CPPFLAGS += -I"$(ODBCHOME)/include" -DUNIX 
CFLAGS += -AA +objdebug -mt +u1 +W 849 +W 641 
LDFLAGS += -L"$(ODBCHOME)/lib" $(CFLAGS)

include ../config/unixODBC.inc
MAKEFILEDEPS += ../config/unixODBC.inc
