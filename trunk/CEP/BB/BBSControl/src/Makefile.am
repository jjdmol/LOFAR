lib_LTLIBRARIES			= libbbscontrol.la

libbbscontrol_la_SOURCES	= \
	BBSCorrectStep.cc \
	BBSMultiStep.cc \
	BBSPredictStep.cc \
	BBSRefitStep.cc \
	BBSShiftStep.cc \
	BBSSingleStep.cc \
	BBSSolveStep.cc \
	BBSStep.cc \
	BBSStrategy.cc \
	BBSStructs.cc \
	BBSSubtractStep.cc \
	BlobStreamableConnection.cc \
	DomainRegistrationRequest.cc \
	CommandQueue.cc \
	CommandQueueTransactors.cc \
	GlobalProcessControl.cc \
	IterationRequest.cc \
	IterationResult.cc \
	KernelProcessControl.cc \
	pqutil.cc \
	SolverProcessControl.cc

bin_PROGRAMS			= GlobalControl KernelControl SolverControl

GlobalControl_SOURCES		= GlobalControl.cc
GlobalControl_LDADD		= libbbscontrol.la
GlobalControl_DEPENDENCIES	= libbbscontrol.la $(LOFAR_DEPEND)

KernelControl_SOURCES		= KernelControl.cc
KernelControl_LDADD		= libbbscontrol.la
KernelControl_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

SolverControl_SOURCES		= SolverControl.cc
SolverControl_LDADD		= libbbscontrol.la
SolverControl_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
