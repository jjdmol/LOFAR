DOCHDRS	=	\
  GPI_Defines.h \
  GPI_Controller.h \
  GPI_SupervisoryServer.h \
  GPI_PropertyProxy.h \
  SS/FProperty.h \
  SS/FPValue.h \
  SS/FPBoolValue.h \
  SS/FPCharValue.h \
  SS/FPIntegerValue.h \
  SS/FPUnsignedValue.h \
  SS/FPDoubleValue.h \
  SS/FPStringValue.h \
  SS/FPDynArrValue.h
    
lib_LTLIBRARIES 	= libpi.la

libpi_la_SOURCES= $(DOCHDRS) \
  GPI_Controller.cc \
  GPI_SupervisoryServer.cc \
  GPI_PropertyProxy.cc \
  SS/FProperty.cc \
  SS/FPValue.cc \
  SS/FPBoolValue.cc \
  SS/FPCharValue.cc \
  SS/FPIntegerValue.cc \
  SS/FPUnsignedValue.cc \
  SS/FPDoubleValue.cc \
  SS/FPStringValue.cc \
  SS/FPDynArrValue.cc

bin_PROGRAMS   		= PropertyInterface
  
PropertyInterface_SOURCES		    = GPI_Main.cc
PropertyInterface_LDADD		      = libpi.la 
PropertyInterface_DEPENDENCIES	= libpi.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	F_Supervisory_Protocol.ph

EXTRA_DIST = \
	F_Supervisory_Protocol.ph \
	mac.ns \
	mac.top \
	log4cplus.properties
	
pkgdata_DATA = \
	mac.ns \
	mac.top \
	log4cplus.properties

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: $(MAC_CONFIG)/%.ns
	cp $< $@

%.top: $(MAC_CONFIG)/%.top
	cp $< $@

include $(lofar_sharedir)/Makefile.common

