SUBDIRS=src

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

EXTRA_DIST = \
      Makefile.common \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
