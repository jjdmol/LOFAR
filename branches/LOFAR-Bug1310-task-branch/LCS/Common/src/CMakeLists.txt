include_directories(${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../include)

if (HAVE_SHMEM)
  add_subdirectory(shmem)
endif (HAVE_SHMEM)

#add_definitions (-DHAVE_BOOST)

#try_compile (HAVE_PRETTY_FUNCTION ${CMAKE_BINARY_DIR} 

########### next target ###############

SET(common_LIB_SRCS
   Allocator.cc
   DataConvert.cc
   Exception.cc
   FileLocator.cc
   hexdump.cc
   KVpair.cc
   lofar_bitset.cc
   LofarLogCout.cc
   LofarLogger.cc
   Numeric.cc
   ParameterSet.cc
   ParameterSetImpl.cc
   ParameterValue.cc
   PrettyUnits.cc
   Process.cc
   RunOnNode.cc
   Stopwatch.cc
   StreamUtil.cc
   StringUtil.cc
   SystemUtil.cc
   Timer.cc
   TypeNames.cc
   Net/FdSet.cc
   Net/Socket.cc
)

add_library(common STATIC ${common_LIB_SRCS})
target_link_libraries(common)
install(TARGETS common DESTINATION lib)

if (HAVE_SHMEM)
  target_link_libraries(shmem)
  install(TARGETS shmem DESTINATION lib)
endif (HAVE_SHMEM)


#add_executable(versioncommon versioncommon.cc)
#target_link_libraries(versioncommon common)
#install(TARGETS versioncommon DESTINATION bin)


add_executable(getparsetvalue getparsetvalue.cc)
target_link_libraries(getparsetvalue common)
install(TARGETS getparsetvalue DESTINATION bin)


########### install files ###############




#original Makefile.am contents follow:

#if HAVE_SHMEM
#SUBDIRS	= shmem .
#endif
#
#if USE_BACKTRACE
#backtrace_SRCS = AddressTranslator.cc Backtrace.cc SymbolTable.cc
#endif
#
#lib_LTLIBRARIES 	= libcommon.la
#
#libcommon_la_SOURCES 	= Package__Version.cc \
#	Allocator.cc \
#	DataConvert.cc \
#	Exception.cc \
#	FileLocator.cc \
#	hexdump.cc \
#	KVpair.cc \
#	lofar_bitset.cc \
#	LofarLogCout.cc \
#	LofarLogger.cc \
#	Numeric.cc \
#	ParameterSet.cc \
#	ParameterSetImpl.cc \
#	ParameterValue.cc \
#	PrettyUnits.cc \
#	Process.cc \
#	RunOnNode.cc \
#	Stopwatch.cc \
#	StreamUtil.cc \
#	StringUtil.cc \
#	SystemUtil.cc \
#	Timer.cc \
#	TypeNames.cc \
#	Version.cc \
#	Net/FdSet.cc \
#	Net/Socket.cc
#
#libcommon_la_SOURCES	+= $(backtrace_SRCS)
#
#if HAVE_SHMEM
#libcommon_la_LIBADD	= shmem/libshmem.la
#endif
#
#bin_PROGRAMS = versioncommon getparsetvalue
#
#versioncommon_SOURCES      = versioncommon.cc
#versioncommon_LDADD        = libcommon.la
#versioncommon_DEPENDENCIES = libcommon.la $(LOFAR_DEPEND)
#
#getparsetvalue_SOURCES      = getparsetvalue.cc
#getparsetvalue_LDADD        = libcommon.la
#getparsetvalue_DEPENDENCIES = libcommon.la $(LOFAR_DEPEND)
#
#include $(top_srcdir)/Makefile.common
