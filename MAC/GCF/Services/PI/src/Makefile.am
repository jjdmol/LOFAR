DOCHDRS	=	\
  GPI_Defines.h \
  GPI_Controller.h \
  GPI_SupervisoryServer.h \
  GPI_PropertyProxy.h \
  SS/FProperty.h \
  SS/FPValue.h \
  SS/FPBoolValue.h
    
lib_LTLIBRARIES 	= libpi.la

libpi_la_SOURCES= $(DOCHDRS) \
  GPI_Controller.cc \
  GPI_SupervisoryServer.cc \
  GPI_PropertyProxy.cc \
  SS/FProperty.cc \
  SS/FPValue.cc \
  SS/FPBoolValue.cc

bin_PROGRAMS   		= PropertyInterface
  
PropertyInterface_SOURCES		    = GPI_Main.cc
PropertyInterface_LDADD		      = libpi.la 
PropertyInterface_DEPENDENCIES	= libpi.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

