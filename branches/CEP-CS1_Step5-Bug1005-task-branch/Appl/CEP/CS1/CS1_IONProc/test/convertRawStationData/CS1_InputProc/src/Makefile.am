lib_LTLIBRARIES				= libcs1_inputproc.la

libcs1_inputproc_la_SOURCES	=         ACCmain_InputProc.cc \
					  InputSection.cc \
					  BeamletBuffer.cc		\
                                          Connector.cc			\
					  InputThread.cc		\
                                          ION_Allocator.cc		\
                                          ReaderWriterSynchronization.cc\
                                          WH_DelayCompensation.cc	\
                                          Package__Version.cc

bin_PROGRAMS 				= CS1_InputProc

CS1_InputProc_SOURCES		= CS1_InputProc_main.cc
CS1_InputProc_LDADD		= ../src/libcs1_inputproc.la
CS1_InputProc_DEPENDENCIES	= ../src/libcs1_inputproc.la \
					  $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

sysconf_DATA = CS1_InputProc.log_prop
