lib_LTLIBRARIES			= libcs1_storage.la

libcs1_storage_la_SOURCES	= AH_Storage.cc           \
				  WH_SubbandWriter.cc     \
				  ACCmain_Storage.cc       \
				  MSWriter.cc             \
				  MSWriterImpl.cc

bin_PROGRAMS			= CS1_Storage

CS1_Storage_SOURCES		= CS1_Storage_main.cc
CS1_Storage_LDADD		= libcs1_storage.la
CS1_Storage_DEPENDENCIES	= libcs1_storage.la $(LOFAR_DEPEND)

configfilesdir			= $(bindir)
configfiles_DATA                = CS1_Storage.machinefile \
				  CS1_Storage.log_prop \
				  typeStruct_TTeraFlopCorrelator.dpl \
				  PropertyInterface.conf

include $(top_srcdir)/Makefile.common

sysconf_DATA = CS1_Storage.log_prop
