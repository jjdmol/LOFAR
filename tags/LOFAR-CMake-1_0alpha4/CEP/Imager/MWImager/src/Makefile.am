lib_LTLIBRARIES		= libmwimager.la

libmwimager_la_SOURCES	= Package__Version.cc \
			  ConvertParset.cc

bin_PROGRAMS 		= versionmwimager \
			  convertimagerparset \
			  combineFacets

convertimagerparset_SOURCES		= convertimagerparset.cc
convertimagerparset_LDADD		= libmwimager.la
convertimagerparset_DEPENDENCIES	= libmwimager.la $(LOFAR_DEPEND)

combineFacets_SOURCES		= combineFacets.cc
combineFacets_LDADD		= libmwimager.la
combineFacets_DEPENDENCIES	= libmwimager.la $(LOFAR_DEPEND)

versionmwimager_SOURCES      = versionmwimager.cc
versionmwimager_LDADD        = libmwimager.la
versionmwimager_DEPENDENCIES = libmwimager.la $(LOFAR_DEPEND)

scriptdir = $(bindir)
dist_script_SCRIPTS = mwimager mwimager-dd \
			mwimager-part mwimager-askap mwimager-casa \
			makeCombinedImage-part

include $(top_srcdir)/Makefile.common

