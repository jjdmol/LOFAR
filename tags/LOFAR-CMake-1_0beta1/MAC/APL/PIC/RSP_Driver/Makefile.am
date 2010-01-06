
SUBDIRS = src test

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

EXTRA_DIST = \
      Makefile.common \
      RSPDriver.spec \
      autoconf_share/compiletool

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix)\
	--with-suite=$(prefix)\
	--with-gcftm=$(prefix)\
	--with-rsp_protocol=$(prefix)

include $(top_srcdir)/Makefile.common

