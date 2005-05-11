bin_PROGRAMS 		= aratest

TESTS			= #aratest

aratest_CPPFLAGS= \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_srcdir)/PIC/RegisterAccess/src \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

aratest_SOURCES 		= \
	ARATestAnswer.cc \
	ARATestTask.cc \
	ARATestDriverTask.cc \
	ARATestMain.cc
	
#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	aratest.conf \
	aratest.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
