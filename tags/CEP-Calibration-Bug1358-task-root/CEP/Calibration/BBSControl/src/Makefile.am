lib_LTLIBRARIES			= libbbscontrol.la

libbbscontrol_la_SOURCES	= Package__Version.cc  \
	AddStep.cc \
	BlobStreamableConnection.cc \
	CalSession.cc \
	CalSessionTransactors.cc \
	Command.cc \
	CommandResult.cc \
	CorrectStep.cc \
	FinalizeCommand.cc \
	GlobalProcessControl.cc \
	GlobalSolveController.cc \
	InitializeCommand.cc \
	KernelConnection.cc \
	KernelProcessControl.cc \
	LocalSolveController.cc \
	Messages.cc \
	MultiStep.cc \
	NextChunkCommand.cc \
	NoiseStep.cc \
	PredictStep.cc \
	RecoverCommand.cc \
	RefitStep.cc \
	ShiftStep.cc \
	SingleStep.cc \
	SolverProcessControl.cc \
	SolveStep.cc \
	SolveTask.cc \
	Step.cc \
	Strategy.cc \
	StreamUtil.cc \
	SubtractStep.cc \
	SynchronizeCommand.cc \
	Types.cc

bin_PROGRAMS			= versionbbscontrol GlobalControl KernelControl SolverControl

GlobalControl_SOURCES		= GlobalControl.cc
GlobalControl_LDADD		= libbbscontrol.la
GlobalControl_DEPENDENCIES	= libbbscontrol.la $(LOFAR_DEPEND)

KernelControl_SOURCES		= KernelControl.cc
KernelControl_LDADD		= libbbscontrol.la
KernelControl_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

SolverControl_SOURCES		= SolverControl.cc
SolverControl_LDADD		= libbbscontrol.la
SolverControl_DEPENDENCIES 	= libbbscontrol.la $(LOFAR_DEPEND)

versionbbscontrol_SOURCES      = versionbbscontrol.cc
versionbbscontrol_LDADD        = libbbscontrol.la
versionbbscontrol_DEPENDENCIES = libbbscontrol.la $(LOFAR_DEPEND)

scriptdir = $(bindir)
dist_script_SCRIPTS = calibrate calibrate-part

include $(top_srcdir)/Makefile.common
