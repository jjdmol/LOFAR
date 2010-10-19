check_PROGRAMS        = tAxis tAxisMapping tBox tGrid \
			tParmValue tParmDBCasa \
			tParmSet tParmCache tParm tParmFacade \
			tSourceDBCasa tTimeAxis

# programs to run through supplied checktools
CHECKTOOLPROGS        = $(check_PROGRAMS)

# scripts used to run tests
TESTSCRIPTS           = tAxis.sh tAxisMapping.sh tBox.sh tGrid.sh \
			tParmValue.sh tParmDBCasa.sh \
			tParmSet.sh tParmCache tParm.sh \
			tParmFacade.sh tParmFacadeDistr.sh \
			tparmdb.sh \
			tSourceDBCasa.sh tmakesourcedb.sh \
			tsetupparmdb.sh tsetupsourcedb.sh \
			tTimeAxis.sh

# scripts and possible programs to run tests
TESTS	              = $(TESTSCRIPTS)

XFAIL_TESTS           =

tAxis_SOURCES			= tAxis.cc
tAxis_LDADD			= ../src/libparmdb.la
tAxis_DEPENDENCIES		= ../src/libparmdb.la $(LOFAR_DEPEND)

tAxisMapping_SOURCES		= tAxisMapping.cc
tAxisMapping_LDADD		= ../src/libparmdb.la
tAxisMapping_DEPENDENCIES	= ../src/libparmdb.la $(LOFAR_DEPEND)

tBox_SOURCES			= tBox.cc
tBox_LDADD			= ../src/libparmdb.la
tBox_DEPENDENCIES		= ../src/libparmdb.la $(LOFAR_DEPEND)

tGrid_SOURCES			= tGrid.cc
tGrid_LDADD			= ../src/libparmdb.la
tGrid_DEPENDENCIES		= ../src/libparmdb.la $(LOFAR_DEPEND)

tParmValue_SOURCES		= tParmValue.cc
tParmValue_LDADD		= ../src/libparmdb.la
tParmValue_DEPENDENCIES		= ../src/libparmdb.la $(LOFAR_DEPEND)

tParmDBCasa_SOURCES		= tParmDBCasa.cc
tParmDBCasa_LDADD		= ../src/libparmdb.la
tParmDBCasa_DEPENDENCIES	= ../src/libparmdb.la $(LOFAR_DEPEND)

tParmSet_SOURCES		= tParmSet.cc
tParmSet_LDADD			= ../src/libparmdb.la
tParmSet_DEPENDENCIES		= ../src/libparmdb.la $(LOFAR_DEPEND)

tParmCache_SOURCES		= tParmCache.cc
tParmCache_LDADD		= ../src/libparmdb.la
tParmCache_DEPENDENCIES		= ../src/libparmdb.la $(LOFAR_DEPEND)

tParm_SOURCES			= tParm.cc
tParm_LDADD			= ../src/libparmdb.la
tParm_DEPENDENCIES		= ../src/libparmdb.la $(LOFAR_DEPEND)

tParmFacade_SOURCES		= tParmFacade.cc
tParmFacade_LDADD		= ../src/libparmdb.la
tParmFacade_DEPENDENCIES	= ../src/libparmdb.la $(LOFAR_DEPEND)

tSourceDBCasa_SOURCES		= tSourceDBCasa.cc
tSourceDBCasa_LDADD		= ../src/libparmdb.la
tSourceDBCasa_DEPENDENCIES	= ../src/libparmdb.la $(LOFAR_DEPEND)

tTimeAxis_SOURCES		= tTimeAxis.cc
tTimeAxis_LDADD			= ../src/libparmdb.la
tTimeAxis_DEPENDENCIES		= ../src/libparmdb.la $(LOFAR_DEPEND)

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST            = $(TESTSCRIPTS) \
			tparmdb.run tparmdb.in tparmdb.stdout \
			tParmFacade.run tParmFacade.stdout \
			tParmFacadeDistr.run tParmFacadeDistr.stdout \
			tmakesourcedb.run tmakesourcedb.in \
			tmakesourcedb.in_2 tmakesourcedb.stdout \
			tsetupparmdb.run \
			tsetupparmdb.in_cd tsetupparmdb.in_gds \
			tsetupparmdb.in_ms0.vds tsetupparmdb.in_ms1.vds \
			tsetupparmdb.in_ms2.vds
			tsetupsourcedb.run tsetupsourcedb.in \
			tsetupsourcedb.in_cd tsetupsourcedb.in_gds \
			tsetupsourcedb.in_ms0.vds tsetupsourcedb.in_ms1.vds \
			tsetupsourcedb.in_ms2.vds


include $(top_srcdir)/Makefile.common
