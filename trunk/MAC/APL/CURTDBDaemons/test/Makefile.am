AM_CPPFLAGS						= -I$(top_builddir)/include

bin_PROGRAMS					= tLoggingProcessor tKeyValueLogger

tLoggingProcessor_SOURCES		= tLoggingProcessor.cc
tLoggingProcessor_LDADD			= $(libdir)/liblog_protocol.a
tLoggingProcessor_DEPENDENCIES	= $(LOFAR_DEPEND) $(libdir)/liblog_protocol.a

tKeyValueLogger_SOURCES			= tKeyValueLogger.cc
tKeyValueLogger_LDADD			= $(libdir)/libkvt_protocol.a $(libdir)/libeventport.a
tKeyValueLogger_DEPENDENCIES	= $(LOFAR_DEPEND) $(libdir)/libkvt_protocol.a $(libdir)/libeventport.a

include $(top_srcdir)/Makefile.common
