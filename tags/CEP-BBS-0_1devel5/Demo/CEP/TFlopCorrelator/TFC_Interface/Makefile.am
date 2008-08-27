SUBDIRS=src test

ACLOCAL_AMFLAGS = -I $(top_srcdir)/autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS=\
      --with-common=$(prefix) \
      --with-transport=$(prefix) \
      --with-aps=$(prefix) \
      --with-tinycep=$(prefix)

include $(top_srcdir)/Makefile.common
