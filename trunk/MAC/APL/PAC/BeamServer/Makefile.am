
SUBDIRS = src test

include $(top_srcdir)/Makefile.common

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix)\
	--with-suite=$(prefix)\
	--with-gcftm=$(prefix)\
	--with-gcfcommon=$(prefix) \
	--with-rsp=$(prefix)

EXTRA_DIST = \
	Makefile.common \
	BeamServer.spec \
	autoconf_share/compiletool

