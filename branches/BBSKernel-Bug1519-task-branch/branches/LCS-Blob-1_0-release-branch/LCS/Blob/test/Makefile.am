check_PROGRAMS	      =	tBlobString tBlobStream tBlobArray tBlobField \
			tKeyValueMap 

# programs to run through supplied checktools
CHECKTOOLPROGS	      =	tBlobString tBlobStream tBlobArray tBlobField \
			tKeyValueMap 
#ENDCHECKTOOLPROGS

TESTSCRIPTS = \
	  tBlobArray_test.sh \
	  tBlobField_test.sh \
          tBlobStream_test.sh \
          tBlobString_test.sh \
	  tKeyValueMap_test.sh 

TESTS	= $(TESTSCRIPTS)

EXTRA_DIST = $(TESTSCRIPTS) \
	  tBlobField.in_be tBlobField.in_le \
          tBlobStream.in_be tBlobStream.in_le tBlobStream.stdout \
	  tKeyValueMap.in tKeyValueMap.stdout 

tBlobString_SOURCES		= tBlobString.cc
tBlobString_LDADD		= ../src/libblob.la
tBlobString_DEPENDENCIES	= ../src/libblob.la $(LOFAR_DEPEND)

tBlobStream_SOURCES		= tBlobStream.cc
tBlobStream_LDADD		= ../src/libblob.la
tBlobStream_DEPENDENCIES	= ../src/libblob.la $(LOFAR_DEPEND)

tBlobArray_SOURCES		= tBlobArray.cc
tBlobArray_LDADD		= ../src/libblob.la
tBlobArray_DEPENDENCIES		= ../src/libblob.la $(LOFAR_DEPEND)

tBlobField_SOURCES		= tBlobField.cc
tBlobField_LDADD		= ../src/libblob.la
tBlobField_DEPENDENCIES		= ../src/libblob.la $(LOFAR_DEPEND)

tKeyValueMap_SOURCES		= tKeyValueMap.cc
tKeyValueMap_LDADD		= ../src/libblob.la
tKeyValueMap_DEPENDENCIES	= ../src/libblob.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT)


include $(top_srcdir)/Makefile.common
