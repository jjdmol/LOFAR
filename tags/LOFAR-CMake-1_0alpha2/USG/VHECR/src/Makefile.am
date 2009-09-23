bin_PROGRAMS 				= VHECRtest

VHECRtest_SOURCES			= VHECRtest.cc \
							  TBBTrigger.cc \
							  TBBReadCmd.cc \
							  VHECRTask.cc
VHECRtest_LDADD				= $(LOFAR_DEPEND)
VHECRtest_DEPENDENCIES		= $(LOFAR_DEPEND)

NOINSTHDRS 					= TBBTrigger.h \
							  TBBReadCmd.h \
							  VHECRTask.h 

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 
	
sysconf_DATA				= VHECRtest.log_prop

include $(top_srcdir)/Makefile.common
