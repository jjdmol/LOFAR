check_PROGRAMS			= tpqxx \
				  tpqutil \
				  tBBSStrategy \
				  tCommandQueue \
				  tQuery

check_SCRIPTS			= tpqutil.sh \
				  tBBSStrategy.sh \
				  tCommandQueue.sh

# programs to run through supplied checktools
CHECKTOOLPROGS			= tpqutil \
				  tBBSStrategy \
				  tCommandQueue
#ENDCHECKTOOLPROGS

tpqxx_SOURCES			= tpqxx.cc
tpqxx_LDADD			= ../src/libbbscontrol.la
tpqxx_DEPENDENCIES		= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tpqutil_SOURCES			= tpqutil.cc
tpqutil_LDADD			= ../src/libbbscontrol.la
tpqutil_DEPENDENCIES		= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tBBSStrategy_SOURCES		= tBBSStrategy.cc
tBBSStrategy_LDADD		= ../src/libbbscontrol.la
tBBSStrategy_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)
EXTRA_tBBSStrategy_DIST		= tBBSStrategy.run tBBSStrategy.stdout

tCommandQueue_SOURCES		= tCommandQueue.cc
tCommandQueue_LDADD		= ../src/libbbscontrol.la
tCommandQueue_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tQuery_SOURCES			= tQuery.cc
tQuery_LDADD			= ../src/libbbscontrol.la
tQuery_DEPENDENCIES		= ../src/libbbscontrol.la $(LOFAR_DEPEND)

TESTS				= $(check_SCRIPTS)

EXTRA_DIST			= $(check_SCRIPTS) \
				  $(EXTRA_tBBSStrategy_DIST) \
				  $(EXTRA_tDH_BBSStep_DIST) \
				  $(EXTRA_tDH_BBSStrategy_DIST) \
				  tBBSControl.parset

include $(top_srcdir)/Makefile.common
