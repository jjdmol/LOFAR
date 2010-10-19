check_PROGRAMS			= tDebug \
                                  tBlobString tBlobHeader tBlobArrayHeader \
                                  tBlobStream tBlobArray tBlobField \
				  tDataConvert tKeyValueMap \
				  tStringUtil 
#				  tLorrays

TESTS	= tDebug_test.sh \
          tBlobString_test.sh \
	  tBlobHeader_test.sh \
	  tBlobArrayHeader_test.sh \
          tBlobStream_test.sh \
	  tBlobArray_test.sh \
	  tBlobField_test.sh \
	  tDataConvert_test.sh \
	  tKeyValueMap_test.sh \
	  tStringUtil_test.sh 
#	  tLorrays_test.sh				

tDebug_SOURCES			= tDebug.cc
tDebug_LDADD			= ../src/libcommon.la
tDebug_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobString_SOURCES		= tBlobString.cc
tBlobString_LDADD		= ../src/libcommon.la
tBlobString_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobHeader_SOURCES		= tBlobHeader.cc
tBlobHeader_LDADD		= ../src/libcommon.la
tBlobHeader_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobArrayHeader_SOURCES	= tBlobArrayHeader.cc
tBlobArrayHeader_LDADD		= ../src/libcommon.la
tBlobArrayHeader_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobStream_SOURCES		= tBlobStream.cc
tBlobStream_LDADD		= ../src/libcommon.la
tBlobStream_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobArray_SOURCES		= tBlobArray.cc
tBlobArray_LDADD		= ../src/libcommon.la
tBlobArray_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobField_SOURCES		= tBlobField.cc
tBlobField_LDADD		= ../src/libcommon.la
tBlobField_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tDataConvert_SOURCES		= tDataConvert.cc
tDataConvert_LDADD		= ../src/libcommon.la
tDataConvert_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tKeyValueMap_SOURCES		= tKeyValueMap.cc
tKeyValueMap_LDADD		= ../src/libcommon.la
tKeyValueMap_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tStringUtil_SOURCES		= tStringUtil.cc
tStringUtil_LDADD		= ../src/libcommon.la	
tStringUtil_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

#tLorrays_SOURCES		= tLorrays.cc
#tLorrays_LDADD			= ../src/libcommon.la	
#tLorrays_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)


include $(lofar_sharedir)/Makefile.common
