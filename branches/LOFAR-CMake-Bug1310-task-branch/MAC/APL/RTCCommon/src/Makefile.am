
AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = 

clean-local:
	rm -f *.ph

EXTRA_DIST 	= $(BUILT_SOURCES) \
			  rc.status

etcdir		= $(prefix)/etc
etc_SCRIPTS = rc.status

sysconf_DATA =

lib_LTLIBRARIES = librtccommon.la libgnuplot_i.la

librtccommon_la_SOURCES   = Package__Version.cc  \
							CableAttenuation.cc \
							RCUCables.cc \
							Timestamp.cc \
							Subject.cc \
							daemonize.c \
							RegisterState.cc

libgnuplot_i_la_SOURCES  = gnuplot_i.cc

bin_PROGRAMS = versionrtccommon

versionrtccommon_SOURCES      = versionrtccommon.cc
versionrtccommon_LDADD        = librtccommon.la
versionrtccommon_DEPENDENCIES = librtccommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
