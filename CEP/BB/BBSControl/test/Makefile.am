check_PROGRAMS			= tBBSStrategy \
				  tDH_BBSStep \
				  tDH_BBSStrategy

check_SCRIPTS			= tBBSStrategy.sh \
				  tDH_BBSStep.sh \
				  tDH_BBSStrategy.sh

# programs to run through supplied checktools
CHECKTOOLPROGS			= tBBSStrategy \
				  tDH_BBSStep \
				  tDH_BBSStrategy
#ENDCHECKTOOLPROGS

tBBSStrategy_SOURCES		= tBBSStrategy.cc
tBBSStrategy_LDADD		= ../src/libbbscontrol.la
tBBSStrategy_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)
EXTRA_tBBSStrategy_DIST		= tBBSStrategy.run tBBSStrategy.stdout

tDH_BBSStep_SOURCES		= tDH_BBSStep.cc
tDH_BBSStep_LDADD		= ../src/libbbscontrol.la
tDH_BBSStep_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)
EXTRA_tDH_BBSStep_DIST		= tDH_BBSStep.run

tDH_BBSStrategy_SOURCES		= tDH_BBSStrategy.cc
tDH_BBSStrategy_LDADD		= ../src/libbbscontrol.la
tDH_BBSStrategy_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)
EXTRA_tDH_BBSStrategy_DIST	= tDH_BBSStrategy.run

TESTS				= $(check_SCRIPTS)

EXTRA_DIST			= $(check_SCRIPTS) \
				  $(EXTRA_tBBSStrategy_DIST) \
				  $(EXTRA_tDH_BBSStep_DIST) \
				  $(EXTRA_tDH_BBSStrategy_DIST) \
				  tBBSControl.parset

include $(top_srcdir)/Makefile.common
