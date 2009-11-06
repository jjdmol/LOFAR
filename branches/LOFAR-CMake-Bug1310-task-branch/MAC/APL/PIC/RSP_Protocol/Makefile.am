
SUBDIRS	= src test include

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix) \
	--with-gcftm=$(prefix) \
	--with-rtccommon=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      RSP_Protocol.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
