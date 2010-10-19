lib_LTLIBRARIES			= libotdb.la

libotdb_la_SOURCES 		= Package__Version.cc  $(DOCHDRS) \
							OTDBconnection.cc	\
							misc.cc \
							OTDBtree.cc	\
							OTDBnode.cc	\
							OTDBvalue.cc	\
							OTDBparam.cc	\
							VICnodeDef.cc \
							Converter.cc \
							wSpaceSplit.cc \
							TreeMaintenance.cc \
							TreeState.cc \
							TreeValue.cc \
							loadCompFile.cc \
							CampaignInfo.cc \
							Campaign.cc

#							OTDBaction.cc	\
#						    OTDBactionList.cc	\
#						    OTDBevent.cc	\
#						    OTDBeventList.cc	

bin_PROGRAMS = versionotdb

versionotdb_SOURCES      = versionotdb.cc
versionotdb_LDADD        = libotdb.la
versionotdb_DEPENDENCIES = libotdb.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
