lib_LTLIBRARIES			= libinterface.la

libinterface_la_SOURCES	        = Package__Version.cc \
				  Allocator.cc \
				  CN_Configuration.cc \
				  CN_Mapping.cc \
				  CN_ProcessingPlan.cc \
				  Parset.cc \
				  PencilCoordinates.cc \
				  RSPTimeStamp.cc \
                                  Thread.cc

configfilesdir			= $(bindir)

bin_PROGRAMS = versioninterface

versioninterface_SOURCES      = versioninterface.cc
versioninterface_LDADD        = libinterface.la
versioninterface_DEPENDENCIES = libinterface.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
