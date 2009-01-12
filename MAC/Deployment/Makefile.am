SUBDIRS=data

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

EXTRA_DIST = \
      Makefile.common \
      Deployment.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
