DOCHDRS	= \
ParameterSet.h 

lib_LTLIBRARIES                 = libacc.la

libacc_la_SOURCES = $(DOCHDRS) \
ParameterSet.cc

bin_PROGRAMS			= tParameterSet tDemoProg tDemoAC

### tParameterSet ###
tParameterSet_SOURCES 		= tParameterSet.cc
tParameterSet_DEPENDENCIES	= $(LOFAR_DEPEND)
tParameterSet_LDADD		= libacc.la

### tDemoProg ###
tDemoProg_SOURCES 		= ProcessController.cc Event.cc Socket.cc DemoProgram.cc
tDemoProg_DEPENDENCIES	        = $(LOFAR_DEPEND)
tDemoProg_LDADD			= libacc.la

### tDemoAC ###
tDemoAC_SOURCES 		= DemoAC.cc ACCProcess.cc ACCProcessList.cc Event.cc Socket.cc
tDemoAC_DEPENDENCIES	        = $(LOFAR_DEPEND)
tDemoAC_LDADD			= libacc.la

include $(lofar_sharedir)/Makefile.common
