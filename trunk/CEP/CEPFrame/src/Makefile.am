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
DataHolder.h					\
Transportable.h					\
PHEX_Antenna.h					\
ParamBlock.h					\
ParamHolder.h					\
ParamValue.h					\
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
TH_File.h					\
Transport.h					\
TransportHolder.h				\
WH_Empty.h					\
WH_Example.h					\
WH_Tester.h					\
WorkHolder.h					\
four1.h						\
realft.h					\
Simul2XML.h					\
Lock.h						\
CyclicBuffer.h					\
DH_Database.h					\
PO_DH_Database.h				\
TH_Database.h

lib_LTLIBRARIES 	= libcepframe.la

libcepframe_la_SOURCES 	= $(DOCHDRS)		\
SimulatorParse.yy				\
SimulatorParse.h				\
SimulatorTokenize.ll				\
SimulatorParseClass.cc				\
DH_Empty.cc					\
DH_Example.cc					\
DH_Tester.cc					\
DataHolder.cc					\
Transportable.cc				\
PHEX_Antenna.cc					\
ParamBlock.cc					\
ParamHolder.cc					\
ParamValue.cc					\
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
Transport.cc					\
TransportHolder.cc				\
WH_Empty.cc					\
WH_Example.cc					\
WH_Tester.cc					\
WorkHolder.cc					\
four1.cc					\
matlabmex.cc					\
realft.cc					\
Simul2XML.cc					\
DH_Database.cc					\
PO_DH_Database.cc				\
TH_Database.cc

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

