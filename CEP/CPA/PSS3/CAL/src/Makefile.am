lib_LTLIBRARIES 	= libcal.la

libcal_la_SOURCES       = MeqCalibraterImpl.cc MeqCalibraterImpl.h

bin_PROGRAMS 		= event meqcalibrater

event_SOURCES		= GlishEvent.cc

meqcalibrater_SOURCES	= meqcalibrater.cc
meqcalibrater_LDADD	= ./libcal.la
meqcalibrater_DEPENDENCIES = ./libcal.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common


AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
