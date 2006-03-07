# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS =nl_astron_lofar_sas_otb_jotdb2_jOTDBConverter.h \
	  nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance.h \
	  nl_astron_lofar_sas_otb_jotdb2_jTreeValue.h \
	  nl_astron_lofar_sas_otb_jotdb2_jCommon.h \
	  nl_astron_lofar_sas_otb_jotdb2_jClassifConv.h \
	  nl_astron_lofar_sas_otb_jotdb2_jParamTypeConv.h \
	  nl_astron_lofar_sas_otb_jotdb2_jTreeStateConv.h \
	  nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv.h \
	  nl_astron_lofar_sas_otb_jotdb2_jUnitConv.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
