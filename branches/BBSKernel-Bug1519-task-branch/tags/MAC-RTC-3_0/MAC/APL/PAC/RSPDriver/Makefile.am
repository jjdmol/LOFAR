SUBDIRS = \
	src \
	test

include $(lofar_sharedir)/Makefile.common

INSTALLDIR=$(HOME)/cvs/HEAD/LOFAR/installed/gnu_debug

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(INSTALLDIR)\
	--with-suite=$(INSTALLDIR)\
	--with-gcftm=$(INSTALLDIR)\
	--with-gcfcommon=$(INSTALLDIR)

DISTCLEANFILES=\
	pkgext\
	pkgextcppflags\
	pkgextcxxflags\
	pkgextobjs\
	lofar_config.old-h\
	lofar_config.h-pkg\
	lofar_config.h\
	.doxygenrc
