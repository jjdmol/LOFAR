check_PROGRAMS 		= tWSRT filluvw

TESTS			= tWSRT filluvw

tWSRT_SOURCES 		= tWSRT.cc 
tWSRT_LDADD		= ../src/libcal.la
tWSRT_DEPENDENCIES	= ../src/libcal.la $(LOFAR_DEPEND)

filluvw_SOURCES 	= filluvw.cc 
filluvw_DEPENDENCIES	= $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

# MPE_LOG_FORMAT=SLOG $(lofar_sharedir)/profiletool tWSRT
profile: tWSRT
	MPIBIN=$(MPIBIN) MPE_LOG_FORMAT=CLOG $(lofar_sharedir)/profiletool tWSRT \
		$(MPIBIN)/mpirun -np 1 ./tWSRT \
		$(lofar_sharedir)/../data/pss1.demo/pss1.demo 0 0 predict 5

