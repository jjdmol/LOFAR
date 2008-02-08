lib_LTLIBRARIES				= libcs1_delaycompensation.la

libcs1_delaycompensation_la_SOURCES	= Package__Version.cc  AH_DelayCompensation.cc \
                                          ACCmain_DelayCompensation.cc \
					  WH_DelayCompensation.cc

bin_PROGRAMS 				= versioncs1_delaycompensation CS1_DelayCompensation

CS1_DelayCompensation_SOURCES		= CS1_DelayCompensation_main.cc
CS1_DelayCompensation_LDADD		= ../src/libcs1_delaycompensation.la
CS1_DelayCompensation_DEPENDENCIES	= ../src/libcs1_delaycompensation.la \
					  $(LOFAR_DEPEND)

versioncs1_delaycompensation_SOURCES      = versioncs1_delaycompensation.cc
versioncs1_delaycompensation_LDADD        = libcs1_delaycompensation.la
versioncs1_delaycompensation_DEPENDENCIES = libcs1_delaycompensation.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

sysconf_DATA = CS1_DelayCompensation.log_prop
