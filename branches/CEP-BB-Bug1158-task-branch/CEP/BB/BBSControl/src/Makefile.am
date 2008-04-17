lib_LTLIBRARIES			= libbbscontrol.la

libbbscontrol_la_SOURCES	= \
	BlobStreamableConnection.cc \
	Command.cc \
	CommandExecutor.cc \
	CommandQueue.cc \
	CommandResult.cc \
	CorrectStep.cc \
	DomainRegistrationRequest.cc \
	FinalizeCommand.cc \
	GlobalProcessControl.cc \
	InitializeCommand.cc \
	IterationRequest.cc \
	IterationResult.cc \
	KernelProcessControl.cc \
	MultiStep.cc \
	NextChunkCommand.cc \
	pqutil.cc \
	PredictStep.cc \
	QueryBuilder/AddCommand.cc \
	RecoverCommand.cc \
	RefitStep.cc \
	ShiftStep.cc \
	SingleStep.cc \
	SolverProcessControl.cc \
	SolveStep.cc \
	Step.cc \
	Strategy.cc \
	StreamUtil.cc \
	Structs.cc \
	SubtractStep.cc \
	SynchronizeCommand.cc

#bin_PROGRAMS			= GlobalControl KernelControl SolverControl

#GlobalControl_SOURCES		= GlobalControl.cc
#GlobalControl_LDADD		= libbbscontrol.la
#GlobalControl_DEPENDENCIES	= libbbscontrol.la $(LOFAR_DEPEND)

#KernelControl_SOURCES		= KernelControl.cc
#KernelControl_LDADD		= libbbscontrol.la
#KernelControl_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

#SolverControl_SOURCES		= SolverControl.cc
#SolverControl_LDADD		= libbbscontrol.la
#SolverControl_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
