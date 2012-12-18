
INCLUDES		 = -I$(top_srcdir)/src -I$(top_srcdir)/config

AM_CFLAGS		 = # -DDEBUG
AM_CPPFLAGS		 = # -DDEBUG

bin_PROGRAMS		 = wrapexec_wrapper
wrapexec_wrapper_SOURCES = wrapexec.cc ArgList.cc ArgList.h
nodist_wrapexec_wrapper_SOURCES = wrapexec_paths.h
wrapexec_wrapper_LDADD	 = $(top_builddir)/config/libconfig.a

wrapexec_paths.h: $(srcdir)/wrapexec_paths.h.in
	sed -e 's%WRAPEXEC_CFG_PATH_PLACEHOLDER%$(sysconfdir)%' < $(srcdir)/wrapexec_paths.h.in > wrapexec_paths.h
wrapexec.cc: wrapexec_paths.h

noinst_DATA         = wrapexec.cfg.in
nodist_sysconf_DATA = wrapexec.cfg

EXTRA_DIST       = wrapexec.cfg.in wrapexec_paths.h.in

wrapexec.cfg: $(srcdir)/wrapexec.cfg.in
	cp $(srcdir)/wrapexec.cfg.in wrapexec.cfg

distclean-local:
	rm -f wrapexec.cfg
	rm -f wrapexec_paths.h
