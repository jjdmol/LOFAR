check_PROGRAMS	      =	tBlobString tBlobStream tBlobArray tBlobSTL tBlobCasa \
			tBlobField tBlobAipsIO tKeyValueMap

# programs to run through supplied checktools
CHECKTOOLPROGS	      =	tBlobString tBlobStream tBlobArray tBlobSTL tBlobCasa \
			tBlobField tBlobAipsIO tKeyValueMap

TESTSCRIPTS = \
          tBlobString_test.sh \
          tBlobStream_test.sh \
	  tBlobArray_test.sh \
	  tBlobSTL_test.sh \
	  tBlobCasa_test.sh \
	  tBlobField_test.sh \
	  tBlobAipsIO_test.sh \
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

tBlobSTL_SOURCES		= tBlobSTL.cc
tBlobSTL_LDADD			= ../src/libblob.la
tBlobSTL_DEPENDENCIES		= ../src/libblob.la $(LOFAR_DEPEND)

tBlobCasa_SOURCES		= tBlobCasa.cc
tBlobCasa_LDADD			= ../src/libblob.la
tBlobCasa_DEPENDENCIES		= ../src/libblob.la $(LOFAR_DEPEND)

tBlobField_SOURCES		= tBlobField.cc
tBlobField_LDADD		= ../src/libblob.la
tBlobField_DEPENDENCIES		= ../src/libblob.la $(LOFAR_DEPEND)

tBlobAipsIO_SOURCES		= tBlobAipsIO.cc
tBlobAipsIO_LDADD		= ../src/libblob.la
tBlobAipsIO_DEPENDENCIES	= ../src/libblob.la $(LOFAR_DEPEND)

tKeyValueMap_SOURCES		= tKeyValueMap.cc
tKeyValueMap_LDADD		= ../src/libblob.la
tKeyValueMap_DEPENDENCIES	= ../src/libblob.la $(LOFAR_DEPEND)



include $(top_srcdir)/Makefile.common
