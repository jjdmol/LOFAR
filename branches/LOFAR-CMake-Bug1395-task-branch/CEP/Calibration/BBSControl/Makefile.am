SUBDIRS = include src test scripts

ACLOCAL_AMFLAGS = -I autoconf_share

pkgextdir     = $(prefix)/config/$(PACKAGE)
pkgext_DATA   = pkgext pkgextcppflags pkgextcxxflags pkgextldflags

DISTCHECK_CONFIGURE_FLAGS = \
      --with-common=$(prefix) \
      --with-blob=$(prefix) \
      --with-transport=$(prefix) \
      --with-transportpostgres=$(prefix) \
      --with-tinycep=$(prefix) \
      --with-cepframe=$(prefix) \
      --with-aps=$(prefix) \
      --with-bbs=$(prefix) \
      --with-parmdb=$(prefix) \
      --with-ms=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      BBSControl.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
