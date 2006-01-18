SUBDIRS=src test include

DISTCHECK_CONFIGURE_FLAGS=\
      --with-common=$(prefix) \
      --with-blob=$(prefix) \
      --with-transport=$(prefix) 

EXTRA_DIST = \
      Makefile.common \
      ALC.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
