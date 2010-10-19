
SUBDIRS	= src test include

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix) \
	--with-gcfcommon=$(prefix) \
	--with-gcftm=$(prefix) \
	--with-rtccommon=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      RSP_Protocol.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
