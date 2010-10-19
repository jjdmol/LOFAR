sbin_PROGRAMS 				= VHECRtest

VHECRtest_SOURCES			= VHECRtest.cc
VHECRtest_LDADD				= ../src/libvhecr.la $(LOFAR_DEPEND)
VHECRtest_DEPENDENCIES		= ../src/libvhecr.la $(LOFAR_DEPEND)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir				= $(bindir)
configfiles_DATA 			= 
sysconf_DATA				= 

include $(top_srcdir)/Makefile.common
