lib_LTLIBRARIES 	= libblob.la

libblob_la_SOURCES 	= \
        BlobArray.cc \
        BlobField2.cc \
        BlobFieldSet.cc \
        BlobHeader.cc \
        BlobIBufChar.cc \
        BlobIBufStream.cc \
        BlobIStream.cc \
        BlobOBufChar.cc \
        BlobOBufNull.cc \
        BlobOBufStream.cc \
        BlobOBufString.cc \
        BlobOStream.cc \
        BlobString.cc \
        BlobStringType.cc \
        KeyParse.yy \
        KeyTokenize.ll \
        KeyParser.cc \
        KeyValue.cc \
        KeyValueMap.cc 

AM_YFLAGS		= -d -p KeyParse
AM_LFLAGS		= -PKeyTokenize
LEX_OUTPUT_ROOT		= lex.KeyTokenize

include $(top_srcdir)/Makefile.common
