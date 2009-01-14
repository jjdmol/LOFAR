SUBDIRS=src test include

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS=\
      --with-common=$(prefix) \
      --with-transport=$(prefix) \
      --with-aps=$(prefix) \
      --with-tinycep=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      Interface.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
