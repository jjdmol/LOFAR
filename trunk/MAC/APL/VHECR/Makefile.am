SUBDIRS=src include

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS=\
      --with-common=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      VHECR.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
