lib_LTLIBRARIES			= libstorage.la

libstorage_la_SOURCES	= Package__Version.cc \
				  InputThread.cc \
				  OutputThread.cc \
				  SubbandWriter.cc \
				  MSWriter.cc      \
				  MSWriterNull.cc  \
				  MSWriterFile.cc  \
                                  MeasurementSetFormat.cc

bin_PROGRAMS			= versionstorage Storage

Storage_SOURCES		        = Storage_main.cc
Storage_LDADD		        = libstorage.la
Storage_DEPENDENCIES	        = libstorage.la $(LOFAR_DEPEND)

configfilesdir			= $(bindir)
configfiles_DATA                = Storage.log_prop \
				  PropertyInterface.conf

versionstorage_SOURCES          = versionstorage.cc
versionstorage_LDADD            = libstorage.la
versionstorage_DEPENDENCIES     = libstorage.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

sysconf_DATA = Storage.log_prop
