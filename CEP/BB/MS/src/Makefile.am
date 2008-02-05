lib_LTLIBRARIES		= libbbms.la

libbbms_la_SOURCES	= DH_MSMake.cc \
			  MSCreate.cc \
			  MSDesc.cc

bin_PROGRAMS 		= MSSplit \
			  makemsdesc \
			  readmsdesc \
			  makems\
			  msplay

MSSplit_SOURCES		= MSSplit.cc
MSSplit_DEPENDENCIES	= $(LOFAR_DEPEND)

makemsdesc_SOURCES	= makemsdesc.cc
makemsdesc_LDADD	= libbbms.la
makemsdesc_DEPENDENCIES	= libbbms.la $(LOFAR_DEPEND)

readmsdesc_SOURCES	= readmsdesc.cc
readmsdesc_LDADD	= libbbms.la
readmsdesc_DEPENDENCIES	= libbbms.la $(LOFAR_DEPEND)

makems_SOURCES		= makems.cc
makems_LDADD		= libbbms.la
makems_DEPENDENCIES	= libbbms.la $(LOFAR_DEPEND)

msplay_SOURCES		= msplay.cc
msplay_LDADD		= libbbms.la
msplay_DEPENDENCIES	= libbbms.la $(LOFAR_DEPEND)

glishdir		= $(libexecdir)/glish
dist_glish_SCRIPTS	= mssplit.g statconv.g

scriptdir		= $(bindir)
dist_script_SCRIPTS	= mssplit

include $(top_srcdir)/Makefile.common

