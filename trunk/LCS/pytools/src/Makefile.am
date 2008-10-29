pkginclude_HEADERS = Package__Version.h

lib_LTLIBRARIES		= libpytools.la

libpytools_la_SOURCES	= Package__Version.cc

bin_PROGRAMS = versionpytools

versionpytools_SOURCES      = versionpytools.cc
versionpytools_LDADD        = libpytools.la
versionpytools_DEPENDENCIES = libpytools.la $(LOFAR_DEPEND)

dist_python_SCRIPTS = __init__.py
pythondir = $(prefix)/lib/python$(PYTHON_VERSION)/site-packages/lofar

include $(top_srcdir)/Makefile.common
