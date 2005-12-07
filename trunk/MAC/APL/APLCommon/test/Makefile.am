noinst_PROGRAMS = \
	ResourceAllocatorTest

ResourceAllocatorTest_CPPFLAGS = \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DTESTBUILD

ResourceAllocatorTest_SOURCES = \
	ResourceAllocatorTest.cc \
	$(top_srcdir)/src/ResourceAllocator.cc

ResourceAllocatorTest_LDADD = 

BUILT_SOURCES = \
	ResourceAllocatorTest.log_prop

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

sysconf_DATA = \
	ResourceAllocatorTest.log_prop

EXTRA_DIST = $(BUILT_SOURCES) $(sysconf_data)

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
