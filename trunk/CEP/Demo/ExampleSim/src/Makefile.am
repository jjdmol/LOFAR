bin_PROGRAMS	= ExampleSim

DOCHDRS 	=				\
ExampleSim.h					\
DH_ExampleSim.h					\
WH_Source.h					\
WH_Multiply.h					\
WH_Add.h

ExampleSim_SOURCES  	= $(DOCHDRS)		\
ExampleSim.cc					\
Simulate.cc					\
DH_ExampleSim.cc				\
WH_Source.cc					\
WH_Multiply.cc					\
WH_Add.cc


ExampleSim_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
