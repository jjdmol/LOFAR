DOCHDRS		 				= \
ParameterSet.h	

bin_PROGRAMS				= tParameterSet tDemoProg tDemoAC

### tParameterSet ###
tParameterSet_SOURCES 		= ParameterSet.cc tParameterSet.cc
tParameterSet_DEPENDENCIES	= $(LOFAR_DEPEND)

### tDemoProg ###
tDemoProg_SOURCES 		= ProcessController.cc Event.cc Socket.cc ParameterSet.cc \
						  DemoProgram.cc
tDemoProg_DEPENDENCIES	= $(LOFAR_DEPEND)

### tDemoAC ###
tDemoAC_SOURCES 		= DemoAC.cc ACCProcess.cc ACCProcessList.cc \
						  Event.cc Socket.cc ParameterSet.cc 
tDemoAC_DEPENDENCIES	= $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
