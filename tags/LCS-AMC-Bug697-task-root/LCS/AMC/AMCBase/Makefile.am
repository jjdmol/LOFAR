SUBDIRS = include src test

DISTCHECK_CONFIGURE_FLAGS = \
	--with-common=$(prefix) \
	--with-blob=$(prefix) \
	--with-transport=$(prefix) 

EXTRA_DIST = \
	Makefile.common \
	AMCBase.spec \
	autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
