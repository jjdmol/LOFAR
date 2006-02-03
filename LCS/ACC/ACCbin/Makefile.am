SUBDIRS=src test include

DISTCHECK_CONFIGURE_FLAGS=\
      --with-common=$(prefix) \
      --with-transport=$(prefix) \
      --with-aps=$(prefix) \
      --with-alc=$(prefix) \
      --with-plc=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      ACCbin.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
