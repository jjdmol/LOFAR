noinst_PROGRAMS		= dPerson \
			  dTransport \
			  dComplex

dPerson_SOURCES		= dPerson.cc \
			  Person.h \
			  Person.cc
dPerson_LDADD		= ../src/libpl.la
dPerson_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

dTransport_SOURCES	= dTransport.cc \
			  Transport.h \
			  Transport.cc
dTransport_LDADD	= ../src/libpl.la
dTransport_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

dComplex_SOURCES	= dComplex.cc \
			  Complex.h
dComplex_LDADD		= ../src/libpl.la
dComplex_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
