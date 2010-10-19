AM_CPPFLAGS					= -I$(top_builddir)/include

bin_PROGRAMS				= LDtestMenu ObservationControl

LDtestMenu_SOURCES			= EventPort.cc  \
							  LDtestMenu.cc
LDtestMenu_DEPENDENCIES		= $(LOFAR_DEPEND)

ObservationControl_SOURCES		= dummyLD.cc
ObservationControl_DEPENDENCIES = $(LOFAR_DEPEND)


include $(top_srcdir)/Makefile.common
