
DOCHDRS		 	= 			\
DH_Empty.h					\
DH_Example.h					\
DH_Tester.h					\
Profiler.h					\
Composite.h						\
NetworkBuilder.h					\
CompositeRep.h					\
VirtualMachine.h				\
ApplicationHolder.h				\
Step.h						\
StepRep.h					\
WH_Empty.h					\
WH_Example.h					\
WH_Tester.h					\
WH_TestAutoTrigger.h				\
Lock.h						\
CyclicBuffer.h					\
DHPoolManager.h					\
CycBufferManager.h				\
SynchronisityManager.h				\
DataManager.h					


lib_LTLIBRARIES 	= libcepframe.la

libcepframe_la_SOURCES 	= $(DOCHDRS)		\
DH_Empty.cc					\
DH_Example.cc					\
DH_Tester.cc					\
Profiler.cc					\
Composite.cc					\
NetworkBuilder.cc					\
CompositeRep.cc					\
VirtualMachine.cc				\
ApplicationHolder.cc				\
Step.cc						\
StepRep.cc					\
WH_Empty.cc					\
WH_Example.cc					\
WH_Tester.cc					\
WH_TestAutoTrigger.cc				\
DHPoolManager.cc				\
CycBufferManager.cc				\
SynchronisityManager.cc				\
DataManager.cc					


EXTRA_DIST = assay

AM_YFLAGS		= -d -p SimulatorParse
AM_LFLAGS		= -PSimulatorTokenize
LEX_OUTPUT_ROOT		= lex.SimulatorTokenize

include $(lofar_sharedir)/Makefile.common

