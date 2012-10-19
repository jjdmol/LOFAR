SUBDIRS=include src test

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

EXTRA_DIST = \
      Makefile.common \
      LACE.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
