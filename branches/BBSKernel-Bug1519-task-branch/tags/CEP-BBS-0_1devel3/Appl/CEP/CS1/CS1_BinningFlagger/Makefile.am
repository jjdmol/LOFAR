SUBDIRS=src test include

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS= \
      --with-common=$(prefix) \
      --with-aips++ 

EXTRA_DIST = \
      Makefile.common \
      CS1_BinningFlagger.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
