SUBDIRS = src include

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix) \
	--with-suite=$(prefix)\
	--with-gcftm=$(prefix)\
	--with-gcfcommon=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      RTCCommon.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common


