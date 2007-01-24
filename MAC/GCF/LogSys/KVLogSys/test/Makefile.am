check_PROGRAMS 			= tKVLlogger

tKVLlogger_SOURCES 		= tKVLlogger.cc
tKVLlogger_LDADD		= ../src/libcepkvlc.la $(libdir)/libeventport.la
tKVLlogger_DEPENDENCIES	= ../src/libcepkvlc.la $(LOFAR_DEPEND)

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common

