if HAVE_SHMEM
SUBDIRS	= shmem .
endif

INSTHDRS = \
        Allocator.h \
        CheckConfig.h \
	ComplexBuiltinFP.h \
	ComplexBuiltinInt.h \
	ComplexC99.h \
	ComplexStdFP.h \
	ComplexStdInt.h \
        DataConvert.h \
        DataFormat.h \
        Exception.h \
        hexdump.h \
        lofar_algorithm.h \
        lofar_complex.h \
        lofar_datetime.h \
        lofar_deque.h \
        lofar_fstream.h \
	lofar_iomanip.h \
        lofar_iosfwd.h \
        lofar_iostream.h \
        lofar_list.h \
        LofarLog4Cplus.h \
        LofarLogCout.h \
        LofarLogger.h \
        lofar_map.h \
	lofar_set.h \
        lofar_sstream.h \
        lofar_stack.h \
        lofar_string.h \
        lofar_thread.h \
        LofarTypedefs.h \
        LofarTypes.h \
        lofar_vector.h \
	Numeric.h \
        RunOnNode.h \
        Stopwatch.h \
        StringUtil.h \
	Timer.h \
        TypeNames.h \
	VectorUtil.h \
	Net/FdSet.h \
	Net/Socket.h 

NOINSTHDRS =

TCCHDRS = \
	TypeNames.tcc

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)


include $(top_srcdir)/Makefile.common
