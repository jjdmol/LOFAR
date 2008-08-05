SUBDIRS = include scripts src demo test

ACLOCAL_AMFLAGS = -I $(top_srcdir)/autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS = \
	--with-common=$(prefix) 

EXTRA_DIST = \
	Makefile.common \
	PL.spec \
	package.dox \
	autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
