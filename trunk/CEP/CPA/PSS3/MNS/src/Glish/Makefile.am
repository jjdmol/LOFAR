
CXXFLAGS 		+= -Wall -Wno-unknown-pragmas -Wno-deprecated -D_GNU_SOURCE

lib_LTLIBRARIES 	= libmns-glish.la libmeqcalibrater.la

libmeqcalibrater_la_SOURCES = MeqCalibraterImpl.cc MeqCalibraterImpl.h

bin_PROGRAMS 		= event meqcalibrater

event_SOURCES		= GlishEvent.cc

meqcalibrater_SOURCES	= meqcalibrater.cc
meqcalibrater_LDADD	= ./libmeqcalibrater.la
meqcalibrater_DEPENDENCIES = ./libmeqcalibrater.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

