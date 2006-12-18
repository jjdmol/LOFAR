SUBDIRS=src test include

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix) \
	--with-gcfcommon=$(prefix) \
	--with-gcftm=$(prefix) \
	--with-rtccommon=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      CAL_Protocol.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
