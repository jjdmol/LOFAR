check_PROGRAMS	      =	tBlobHeader tBlobString tBlobStream tBlobArray \
			tBlobSTL tBlobCasa \
			tBlobField tBlobAipsIO tKeyValueMap

# programs to run through supplied checktools
CHECKTOOLPROGS	      =	$(check_PROGRAMS)

TESTSCRIPTS = \
          tBlobHeader.sh \
          tBlobString.sh \
          tBlobStream.sh \
	  tBlobArray.sh \
	  tBlobSTL.sh \
	  tBlobCasa.sh \
	  tBlobField.sh \
	  tBlobAipsIO.sh \
	  tKeyValueMap.sh 

TESTS	= $(TESTSCRIPTS)

EXTRA_DIST = $(TESTSCRIPTS) \
	  tBlobField.in_be tBlobField.in_le \
          tBlobStream.in_be tBlobStream.in_le tBlobStream.stdout \
	  tKeyValueMap.in tKeyValueMap.stdout 

tBlobHeader_SOURCES		= tBlobHeader.cc
tBlobHeader_LDADD		= ../src/libblob.la
tBlobHeader_DEPENDENCIES	= ../src/libblob.la $(LOFAR_DEPEND)

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
