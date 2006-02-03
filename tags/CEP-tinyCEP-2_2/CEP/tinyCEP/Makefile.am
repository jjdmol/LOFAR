SUBDIRS=src test

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix) \
	--with-transport=$(prefix) \
	--with-aps=$(prefix) \
	--with-plc=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      tinyCEP.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
