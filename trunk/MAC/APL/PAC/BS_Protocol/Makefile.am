SUBDIRS=src test include

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(prefix) \
	--with-gcfcommon=$(prefix) \
	--with-gcftm=$(prefix) \
	--with-rtccommon=$(prefix) \
	--with-rsp_protocol=$(prefix)

EXTRA_DIST = \
      Makefile.common \
      BS_Protocol.spec \
      autoconf_share/compiletool

include $(top_srcdir)/Makefile.common
