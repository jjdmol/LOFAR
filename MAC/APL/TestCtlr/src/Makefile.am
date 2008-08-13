
bin_PROGRAMS 				= versiontestctlr ControllerMenu TestController CodeLoggingTest

ControllerMenu_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
							  -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

ControllerMenu_SOURCES		= Package__Version.cc \
							  ControllerMenu.cc \
							  ControllerMenuMain.cc
ControllerMenu_LDADD		= $(LOFAR_DEPEND)
ControllerMenu_DEPENDENCIES	= $(LOFAR_DEPEND)

TestController_SOURCES		= Package__Version.cc \
							  TestController.cc \
							  TestControllerMain.cc
TestController_LDADD		= $(LOFAR_DEPEND)
TestController_DEPENDENCIES	= $(LOFAR_DEPEND)

CodeLoggingTest_SOURCES		= Package__Version.cc \
							  CodeLoggingTest.cc
CodeLoggingTest_LDADD		= $(LOFAR_DEPEND)
CodeLoggingTest_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= ControllerMenu.h 

INSTHDRS 					= 

pkginclude_HEADERS 			= Package__Version.h  $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= 

#in case of make install these files will be copied to the bindir beside the test apps

sysconf_DATA		= ControllerMenu.log_prop TestController.log_prop CodeLoggingTest.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph

versiontestctlr_SOURCES      = versiontestctlr.cc Package__Version.cc
versiontestctlr_LDADD        = 
versiontestctlr_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
