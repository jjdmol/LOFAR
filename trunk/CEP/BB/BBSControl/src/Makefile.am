lib_LTLIBRARIES		= libbbscontrol.la

libbbscontrol_la_SOURCES= \
	BBSCorrectStep.cc \
	BBSKernelProcessControl.cc \
	BBSMultiStep.cc \
	BBSPredictStep.cc \
	BBSProcessControl.cc \
	BBSRefitStep.cc \
	BBSShiftStep.cc \
	BBSSingleStep.cc \
	BBSSolveStep.cc \
	BBSStep.cc \
	BBSStrategy.cc \
	BBSStructs.cc \
	BBSSubtractStep.cc \
	BBSSolverProcessControl.cc \
	BlobStreamableConnection.cc \
	DomainRegistrationRequest.cc \
	CommandQueue.cc \
	CommandQueueTransactors.cc \
	IterationRequest.cc \
	IterationResult.cc \
	pqutil.cc

bin_PROGRAMS		= BBSControl BBSKernel BBSSolver

BBSControl_SOURCES	= BBSControl.cc
BBSControl_LDADD	= libbbscontrol.la
BBSControl_DEPENDENCIES = libbbscontrol.la $(LOFAR_DEPEND)

BBSKernel_SOURCES 	= BBSKernel.cc
BBSKernel_LDADD 	= libbbscontrol.la
BBSKernel_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

BBSSolver_SOURCES 	= BBSSolver.cc
BBSSolver_LDADD 	= libbbscontrol.la
BBSSolver_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
