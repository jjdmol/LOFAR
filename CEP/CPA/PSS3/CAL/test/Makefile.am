check_PROGRAMS 		= tWSRT filluvw

TESTS			= tWSRT filluvw

tWSRT_SOURCES 		= tWSRT.cc 
tWSRT_DEPENDENCIES	= $(LOFAR_DEPEND)

filluvw_SOURCES 	= filluvw.cc 
filluvw_DEPENDENCIES	= $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

profile: tWSRT
	MPE_LOG_FORMAT=SLOG $(lofar_sharedir)/profiletool tWSRT \
		$(MPIBIN)/mpirun -np 1 ./tWSRT \
		$(lofar_sharedir)/../data/pss1.demo/pss1.demo.MS \
		$(lofar_sharedir)/../data/pss1.demo/pss1.demo.MEP \
		$(lofar_sharedir)/../data/pss1.demo/pss1.demo.GSM
