lib_LTLIBRARIES		= liblofar_impl.la

liblofar_impl_la_SOURCES = $(DOCHDRS) \
BGL_Personality.cc		\
CS1_ION_main.cc		 	\
ION_Allocator.cc		\
AH_ION_Scatter.cc		\
WH_ION_Scatter.cc		\
AH_ION_Gather.cc		\
WH_ION_Gather.cc		\
TH_ZoidServer.cc

install-exec-local:
	ln -sf .libs/liblofar_impl.so lofar_impl.so

uninstall-local:
	rm lofar_impl.so

include $(top_srcdir)/Makefile.common
