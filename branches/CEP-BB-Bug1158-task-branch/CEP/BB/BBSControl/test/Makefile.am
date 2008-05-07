check_PROGRAMS			= tpqutil \
				  tStrategy \
				  tCommandQueue \
				  tClient \
				  tServer

check_SCRIPTS			= tpqutil.sh \
				  tStrategy.sh \
				  tCommandQueue.sh

# programs to run through supplied checktools
CHECKTOOLPROGS			= tpqutil \
				  tStrategy \
				  tCommandQueue
#ENDCHECKTOOLPROGS

tpqutil_SOURCES			= tpqutil.cc
tpqutil_LDADD			= ../src/libbbscontrol.la
tpqutil_DEPENDENCIES		= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tStrategy_SOURCES		= tStrategy.cc
tStrategy_LDADD			= ../src/libbbscontrol.la
tStrategy_DEPENDENCIES		= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tCommandQueue_SOURCES		= tCommandQueue.cc
tCommandQueue_LDADD		= ../src/libbbscontrol.la
tCommandQueue_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tClient_SOURCES			= MyDH.h MyDH.cc tClient.cc
tClient_LDADD			= ../src/libbbscontrol.la
tClient_DEPENDENCIES		= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tServer_SOURCES			= MyDH.h MyDH.cc tServer.cc
tServer_LDADD			= ../src/libbbscontrol.la
tServer_DEPENDENCIES		= ../src/libbbscontrol.la $(LOFAR_DEPEND)

TESTS				= $(check_SCRIPTS)

EXTRA_DIST			= $(check_SCRIPTS) \
				  tStrategy.run tStrategy.stdout \
				  tBBSControl.parset

include $(top_srcdir)/Makefile.common
