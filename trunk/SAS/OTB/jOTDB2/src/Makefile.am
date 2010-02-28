lib_LTLIBRARIES         = libjotdb2.la

libjotdb2_la_SOURCES   = $(DOCHDRS) \
		         nl_astron_lofar_sas_otb_jotdb2_jCommon.cc \
		         nl_astron_lofar_sas_otb_jotdb2_jCampaign.cc \
		         nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection.cc \
		         nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance.cc \
			 nl_astron_lofar_sas_otb_jotdb2_jClassifConv.cc \
			 nl_astron_lofar_sas_otb_jotdb2_jParamTypeConv.cc \
			 nl_astron_lofar_sas_otb_jotdb2_jTreeStateConv.cc \
			 nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv.cc \
			 nl_astron_lofar_sas_otb_jotdb2_jUnitConv.cc \
			 nl_astron_lofar_sas_otb_jotdb2_jInitCPPLogger.cc \
			 nl_astron_lofar_sas_otb_jotdb2_jTreeValue.cc

# AM_YFLAGS               = -d -p KeyParse
# AM_LFLAGS               = -PKeyTokenize
# LEX_OUTPUT_ROOT         = lex.KeyTokenize

# if HAVE_SHMEM
# libjotdb2_la_LIBADD     = shmem/libshmem.la
# endif

bin_PROGRAMS =

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
