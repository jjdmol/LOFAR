lib_LTLIBRARIES		= libmwimager.la

libmwimager_la_SOURCES	= Package__Version.cc \
			  ConvertParset.cc

bin_PROGRAMS 		= versionmwimager \
			  convertimagerparset

convertimagerparset_SOURCES		= convertimagerparset.cc
convertimagerparset_LDADD		= libmwimager.la
convertimagerparset_DEPENDENCIES	= libmwimager.la $(LOFAR_DEPEND)

versionmwimager_SOURCES      = versionmwimager.cc
versionmwimager_LDADD        = libmwimager.la
versionmwimager_DEPENDENCIES = libmwimager.la $(LOFAR_DEPEND)

scriptdir = $(bindir)
dist_script_SCRIPTS = mwimager mwimager-part

include $(top_srcdir)/Makefile.common

