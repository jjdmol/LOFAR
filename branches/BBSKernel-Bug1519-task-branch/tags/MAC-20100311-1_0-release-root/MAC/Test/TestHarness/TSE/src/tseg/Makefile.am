
DOCHDRS = \
	bsegui.h \
	ini_parser.h

bin_PROGRAMS = tse

tse_SOURCES = bsegui.c ini_parser.c $(DOCHDRS)
tse_LDADD		= \
	$(top_builddir)/src/tsek/libtsek.la \
	$(top_builddir)/src/tsed/libtsed.la \
	$(top_builddir)/src/dbg/libdbg.la
	
tse_CFLAGS = \
	-DDEBUG_AUTOTEST \
	-D_NO_CONTROL_PANEL_ \
	-DARITHMETRIC \
	-I$(top_srcdir)/src/tsek \
	-I$(top_srcdir)/src/tsed \
	-I$(top_srcdir)/src/dbg

#in case of make install these files will be copied to the bindir beside the test apps
#configfilesdir=$(bindir)

EXTRA_DIST = \
	tse.ini.in

sysconf_DATA = \
	tse.ini

%.ini: %.ini.in 
	cp $< $@

include $(top_srcdir)/Makefile.common
