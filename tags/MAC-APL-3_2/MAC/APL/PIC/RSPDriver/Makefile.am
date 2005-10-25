
SUBDIRS = src test

EXTRA_DIST = \
      Makefile.common \
      RSPDriver.spec \
      autoconf_share/compiletool

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix)\
	--with-suite=$(prefix)\
	--with-gcftm=$(prefix)\
	--with-gcfcommon=$(prefix) \
	--with-rsp_protocol=$(prefix)

include $(top_srcdir)/Makefile.common

