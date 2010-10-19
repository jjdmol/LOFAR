noinst_PROGRAMS		= dComplex \
			  dPerson \
			  dTransport

dComplex_SOURCES	= dComplex.cc Complex.h
dComplex_LDADD		= ../src/libpl.la
dComplex_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_dComplex_DIST	= PO_Complex.h PO_Complex.tcc \
			  Complex.plmap PO_Complex.sql

dPerson_SOURCES		= dPerson.cc Person.h Person.cc
dPerson_LDADD		= ../src/libpl.la
dPerson_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_dPerson_DIST	= PO_Person.h PO_Person.tcc \
			  Person.plmap PO_Person.sql

dTransport_SOURCES	= dTransport.cc Transport.h Transport.cc
dTransport_LDADD	= ../src/libpl.la
dTransport_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_dTransport_DIST	= PO_Transport.h PO_Transport.tcc \
			  Transport.plmap PO_Transport.sql \
			  dTransport.cc.1 Transport.h.1 \
			  PO_Transport.h.1 PO_Transport.tcc.1 \
			  Transport.plmap.1 PO_Transport.sql.1 \
			  dTransport.cc.2 Transport.h.2 \
			  PO_Transport.h.2 PO_Transport.tcc.2 \
			  Transport.plmap.2 PO_Transport.sql.2 \
			  dTransport.cc.3 Transport.h.3 \
			  PO_Transport.h.3 PO_Transport.tcc.3 \
			  Transport.plmap.3 PO_Transport.sql.3

EXTRA_DIST		= $(EXTRA_dComplex_DIST) \
			  $(EXTRA_dPerson_DIST) \
			  $(EXTRA_dTransport_DIST)

include $(top_srcdir)/Makefile.common
