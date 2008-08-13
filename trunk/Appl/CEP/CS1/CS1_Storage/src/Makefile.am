lib_LTLIBRARIES			= libcs1_storage.la

libcs1_storage_la_SOURCES	= Package__Version.cc \
				  SubbandWriter.cc \
				  MSWriter.cc \
				  MSWriterImpl.cc

bin_PROGRAMS			= versioncs1_storage CS1_Storage

CS1_Storage_SOURCES		= CS1_Storage_main.cc
CS1_Storage_LDADD		= libcs1_storage.la
CS1_Storage_DEPENDENCIES	= libcs1_storage.la $(LOFAR_DEPEND)

configfilesdir			= $(bindir)
configfiles_DATA                = CS1_Storage.machinefile \
				  CS1_Storage.log_prop \
				  PropertyInterface.conf

versioncs1_storage_SOURCES      = versioncs1_storage.cc
versioncs1_storage_LDADD        = libcs1_storage.la
versioncs1_storage_DEPENDENCIES = libcs1_storage.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

sysconf_DATA = CS1_Storage.log_prop
