lib_LTLIBRARIES         = libfcnp_cn.la libfcnp_ion.la

pkginclude_HEADERS	= Package__Version.h fcnp_cn.h fcnp_ion.h protocol.h

libfcnp_cn_la_SOURCES	= Package__Version.cc fcnp_cn.cc
libfcnp_ion_la_SOURCES	= Package__Version.cc fcnp_ion.cc

bin_PROGRAMS		= versionfcnp

versionfcnp_SOURCES	= versionfcnp.cc
versionfcnp_LDADD	= libfcnp_cn.la libfcnp_ion.la
versionfcnp_DEPENDENCIES= libfcnp_ion.la libfcnp_ion.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
