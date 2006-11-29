SUBDIRS=src test include

DISTCHECK_CONFIGURE_FLAGS=\
      --with-common=$(prefix) \
      --with-blob=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      Transport.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
