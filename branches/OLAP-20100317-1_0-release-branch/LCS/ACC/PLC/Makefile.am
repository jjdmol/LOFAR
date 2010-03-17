SUBDIRS=src include

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS=\
      --with-common=$(prefix) \
      --with-blob=$(prefix) \
      --with-transport=$(prefix) \
      --with-aps=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      PLC.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
