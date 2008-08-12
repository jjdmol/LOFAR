SUBDIRS=src

ACLOCAL_AMFLAGS = -I $(top_srcdir)/autoconf_share

DISTCHECK_CONFIGURE_FLAGS=\
      --with-common=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      CS1_IONProc.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
