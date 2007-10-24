lib_LTLIBRARIES				= libcs1_delaycompensation.la

libcs1_delaycompensation_la_SOURCES	= AH_DelayCompensation.cc \
                                          ACCmain_DelayCompensation.cc \
					  WH_DelayCompensation.cc

bin_PROGRAMS 				= CS1_DelayCompensation

CS1_DelayCompensation_SOURCES		= CS1_DelayCompensation_main.cc
CS1_DelayCompensation_LDADD		= ../src/libcs1_delaycompensation.la
CS1_DelayCompensation_DEPENDENCIES	= ../src/libcs1_delaycompensation.la \
					  $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

sysconf_DATA = CS1_DelayCompensation.log_prop
