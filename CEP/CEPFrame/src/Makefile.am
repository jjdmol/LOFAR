if HAVE_CORBA
SUBDIRS			= ShMem Corba . CorbaExe
else
SUBDIRS			= ShMem .
endif

DOCHDRS		 	= 			\
BaseSim.h					\
SimulatorParseClass.h				\
DH_Empty.h					\
DH_Example.h					\
DH_Tester.h					\
DH_Postgresql.h					\
DH_Database.h					\
DataHolder.h					\
Transportable.h					\
PHEX_Antenna.h					\
ParamHolder.h					\
Profiler.h					\
Simul.h						\
SimulBuilder.h					\
SimulRep.h					\
VirtualMachine.h				\
Simulator.h					\
Step.h						\
StepRep.h					\
TH_MPI.h					\
TH_Mem.h					\
TH_Mem_Bl.h					\
TH_File.h					\
BaseTransport.h					\
Transport.h					\
ParamTransport.h				\
TransportHolder.h				\
WH_Empty.h					\
WH_Example.h					\
WH_Tester.h					\
WH_TestAutoTrigger.h				\
WorkHolder.h					\
four1.h						\
realft.h					\
Simul2XML.h					\
Lock.h						\
CyclicBuffer.h					\
TH_Database.h					\
CyclicOWBuffer.h				\
DHPoolManager.h					\
CycBufferManager.h				\
SynchronisityManager.h				\
DataManager.h					\
ParamTransportManager.h				\
ParamManager.h					\
PH_Int.h					\
Selector.h					\
Sel_RoundRobin.h				\
Sel_Linked.h					


lib_LTLIBRARIES 	= libcepframe.la

libcepframe_la_SOURCES 	= $(DOCHDRS)		\
SimulatorParse.yy				\
SimulatorParse.h				\
SimulatorTokenize.ll				\
SimulatorParseClass.cc				\
DH_Empty.cc					\
DH_Example.cc					\
DH_Tester.cc					\
DH_Postgresql.cc				\
DH_Database.cc					\
DataHolder.cc					\
Transportable.cc				\
PHEX_Antenna.cc					\
ParamHolder.cc					\
Profiler.cc					\
Simul.cc					\
SimulBuilder.cc					\
SimulRep.cc					\
VirtualMachine.cc				\
Simulator.cc					\
Step.cc						\
StepRep.cc					\
TH_MPI.cc					\
TH_File.cc					\
TH_Mem.cc					\
TH_Mem_Bl.cc					\
BaseTransport.cc				\
Transport.cc					\
ParamTransport.cc				\
TransportHolder.cc				\
WH_Empty.cc					\
WH_Example.cc					\
WH_Tester.cc					\
WH_TestAutoTrigger.cc				\
WorkHolder.cc					\
four1.cc					\
matlabmex.cc					\
realft.cc					\
Simul2XML.cc					\
TH_Database.cc					\
DHPoolManager.cc				\
CycBufferManager.cc				\
SynchronisityManager.cc				\
DataManager.cc					\
ParamTransportManager.cc			\
ParamManager.cc					\
PH_Int.cc					\
Selector.cc					\
Sel_RoundRobin.cc				\
Sel_Linked.cc					


EXTRA_DIST = assay

AM_YFLAGS		= -d -p SimulatorParse
AM_LFLAGS		= -PSimulatorTokenize
LEX_OUTPUT_ROOT		= lex.SimulatorTokenize

if HAVE_CORBA
libcepframe_la_LIBADD = ShMem/libth_shmem.la Corba/libth_corba.la -lpthread
else
libcepframe_la_LIBADD = ShMem/libth_shmem.la -lpthread
endif

include $(lofar_sharedir)/Makefile.common

