check_PROGRAMS 		= tCAL filluvw

TESTS			= tCAL filluvw

tCAL_SOURCES 		= tCAL.cc 
tCAL_LDADD		= ../src/libcal.la
tCAL_DEPENDENCIES	= ../src/libcal.la $(LOFAR_DEPEND)

filluvw_SOURCES 	= filluvw.cc 
filluvw_DEPENDENCIES	= $(LOFAR_DEPEND)

#tuvw_SOURCES	 	= tuvw.cc 
#tuvw_DEPENDENCIES	= $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

profile: tCAL
	MPIBIN=$(MPIBIN) MPE_LOG_FORMAT=CLOG $(lofar_sharedir)/profiletool tCAL \
		$(MPIBIN)/mpirun -np 1 ./tCAL \
		$(lofar_sharedir)/../data/pss1.demo/pss1.demo 0 0 LOFAR predict 5

profile2:
	MPIBIN=$(MPIBIN) MPE_LOG_FORMAT=CLOG $(lofar_sharedir)/profiletool tCAL \
		$(MPIBIN)/mpirun -np 1 ./tCAL \
		$(lofar_sharedir)/../data/test/CAL/10206365/10206365 0 0 LOFAR predict 5

profile3:
	$(lofar_sharedir)/profiletool tCAL \
		./tCAL \
		$(lofar_sharedir)/../data/test/CAL/10206365/10206365 0 0 LOFAR predict 5

profile4:
	$(lofar_sharedir)/profiletool tCAL \
		./tCAL \
		$(lofar_sharedir)/../data/test/CAL/10206365/10206365 0 0 LOFAR predict 1



AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
