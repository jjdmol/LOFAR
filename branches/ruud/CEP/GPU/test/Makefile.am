DOCHDRS		 				= 

bin_PROGRAMS				= FTtest1 HPCorrelator

### FTtest1 ###
FTtest1_SOURCES 			= FTtest1.cc
FTtest1_LDADD				= ../src/libGPU.a
FTtest1_DEPENDENCIES		= ../src/libGPU.a $(LOFAR_DEPEND)
FTtest1_LFLAGS 				= -lX11 -lXmu -lXt -lGL -lGLU -lCg -lCgGL -lpthread

### HPCorrelator ###
HPCorrelator_SOURCES 		= HPCorrelator.cc 
HPCorrelator_LDADD			= ../src/libGPU.a
HPCorrelator_DEPENDENCIES	= ../src/libGPU.a $(LOFAR_DEPEND)
HPCorrelator_LFLAGS 		= -lX11 -lXmu -lXt -lGL -lGLU -lCg -lCgGL -lpthread

include $(lofar_sharedir)/Makefile.common
