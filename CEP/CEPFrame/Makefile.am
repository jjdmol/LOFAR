SUBDIRS=src test

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS=\
        --with-common=$(prefix) \
        --with-transport=$(prefix) \
        --with-tinycep=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      CEPFrame.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
