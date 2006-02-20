SUBDIRS = include src test

DISTCHECK_CONFIGURE_FLAGS = \
	--with-common=$(prefix) \
	--with-transport=$(prefix) \
	--with-amcbase=$(prefix) 

EXTRA_DIST = \
	Makefile.common \
	AMCImpl.spec \
	autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
