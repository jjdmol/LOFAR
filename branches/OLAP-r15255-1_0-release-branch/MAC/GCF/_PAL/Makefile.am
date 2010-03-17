SUBDIRS=SAL PA PML PI include

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

#DISTCHECK_CONFIGURE_FLAGS=\
#      --with-common=$(prefix) # "common" is an example

EXTRA_DIST = \
      Makefile.common \
      GCFPAL.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
