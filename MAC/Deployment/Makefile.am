SUBDIRS=data

ACLOCAL_AMFLAGS = -I $(top_srcdir)/autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

EXTRA_DIST = \
      Makefile.common \
      Deployment.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
