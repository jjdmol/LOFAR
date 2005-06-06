
INSTHDRS		 	= 			\
DH_Example.h					\
DH_Tester.h					\
NetworkBuilder.h				\
VirtualMachine.h				\
ApplicationHolder.h				\
Step.h						\
StepRep.h					\
WH_Example.h					\
WH_Tester.h					\
DHPoolManager.h					\
CycBufferManager.h				\
SynchronisityManager.h				\
DataManager.h				\
Block.h 					\
BlockRep.h					\
Composite.h					\
CompositeRep.h					

NOINSTHDRS =

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libcepframe.la

libcepframe_la_SOURCES 	= $(DOCHDRS)		\
DH_Example.cc					\
DH_Tester.cc					\
NetworkBuilder.cc				\
VirtualMachine.cc				\
ApplicationHolder.cc				\
Step.cc						\
StepRep.cc					\
WH_Example.cc					\
WH_Tester.cc					\
DHPoolManager.cc				\
CycBufferManager.cc				\
SynchronisityManager.cc				\
DataManager.cc					\
Block.cc					\
BlockRep.cc					\
Composite.cc					\
CompositeRep.cc					

EXTRA_DIST = assay

AM_YFLAGS		= -d -p SimulatorParse
AM_LFLAGS		= -PSimulatorTokenize
LEX_OUTPUT_ROOT		= lex.SimulatorTokenize

include $(lofar_sharedir)/Makefile.common
