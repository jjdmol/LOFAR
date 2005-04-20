bin_PROGRAMS = MSSplit MSDesc

MSSplit_SOURCES = MSSplit.cc
MSSplit_DEPENDENCIES = $(LOFAR_DEPEND)

MSDesc_SOURCES = MSDesc.cc
MSDesc_DEPENDENCIES = $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = mssplit.g statconv.g

scriptdir = $(bindir)
dist_script_SCRIPTS = mssplit

include $(lofar_sharedir)/Makefile.common

