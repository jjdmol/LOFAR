SUBDIRS=src test

DISTCHECK_CONFIGURE_FLAGS = \
	--with-parmdb=$(prefix) \
	--with-ms=$(prefix)

EXTRA_DIST = \
	Makefile.common \
	BBSKernel.spec \
	package.dox \
	autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
