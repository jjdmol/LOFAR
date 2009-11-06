check_PROGRAMS	      =	tStationInfo

# programs to run through supplied checktools
CHECKTOOLPROGS 	      =	tStationInfo

#ENDCHECKTOOLPROGS

TESTSCRIPTS = 

TESTS = $(TESTSCRIPTS)

EXTRA_DIST = $(TESTSCRIPTS) 
#	tComplex.stdout 

tStationInfo_SOURCES		= tStationInfo.cc
tStationInfo_LDADD			= ../src/libdeployment.la	
tStationInfo_DEPENDENCIES	= ../src/libdeployment.la $(LOFAR_DEPEND)




include $(top_srcdir)/Makefile.common
