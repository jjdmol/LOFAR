check_PROGRAMS			= tStrategy \
				  tCommandQueue

check_SCRIPTS			= tStrategy.sh \
				  tCommandQueue.sh

# programs to run through supplied checktools
CHECKTOOLPROGS			= tStrategy \
				  tCommandQueue
#ENDCHECKTOOLPROGS

tStrategy_SOURCES		= tStrategy.cc
tStrategy_LDADD			= ../src/libbbscontrol.la
tStrategy_DEPENDENCIES		= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tCommandQueue_SOURCES		= tCommandQueue.cc
tCommandQueue_LDADD		= ../src/libbbscontrol.la
tCommandQueue_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)

TESTS				= $(check_SCRIPTS)

EXTRA_DIST			= $(check_SCRIPTS) \
				  tStrategy.run tStrategy.stdout \
				  tBBSControl.parset

include $(top_srcdir)/Makefile.common
