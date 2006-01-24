check_PROGRAMS		= tA \
			  tC \
			  tExpr \
			  tAttrib \
			  tCollection \
			  tObjectId \
			  tAttr1 \
			  tQuery

check_SCRIPTS		= tA_test.sh \
			  tC_test.sh \
			  tExpr.sh \
			  tAttrib.sh \
			  tObjectId.sh \
			  tCollection.sh \
			  tAttr1.sh \
			  tQuery.sh

# programs to run through a checktool
CHECKTOOLPROGS          = tExpr \
			  tAttrib \
			  tCollection \
			  tObjectId \
			  tAttr1 \
			  tQuery
#ENDCHECKTOOLPROGS

tA_SOURCES              = tA.cc A.h B.h
tA_LDADD		= ../src/libpl.la
tA_DEPENDENCIES		= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_tA_DIST		= PO_A.h PO_A.tcc PO_A.sql \
			  PO_B.h PO_B.tcc PO_B.sql \
			  tA.run tA.in_sqld tA.stdout \
			  A.plmap PO_A_create_databases.sql

tC_SOURCES              = tC.cc C.h
tC_LDADD		= ../src/libpl.la
tC_DEPENDENCIES		= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_tC_DIST		= PO_C.h PO_C.tcc PO_C.sql \
			  tC.run tC.in_sqld tC.stdout \
			  C.plmap PO_C_create_databases.sql

tExpr_SOURCES		= tExpr.cc
tExpr_LDADD		= ../src/libpl.la
tExpr_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_tExpr_DIST	= tExpr.stdout

tAttrib_SOURCES		= tAttrib.cc tAttrib.h
tAttrib_LDADD		= ../src/libpl.la
tAttrib_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_tAttrib_DIST	= PO_tAttrib.h PO_tAttrib.tcc tAttrib.stdout

tCollection_SOURCES	= tCollection.cc
tCollection_LDADD	= ../src/libpl.la
tCollection_DEPENDENCIES= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_tCollection_DIST	= tCollection.stdout

tObjectId_SOURCES	= tObjectId.cc
tObjectId_LDADD		= ../src/libpl.la
tObjectId_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_tObjectId_DIST	= tObjectId.stdout

tAttr1_SOURCES		= tAttr1.cc tAttr1.h
tAttr1_LDADD		= ../src/libpl.la
tAttr1_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_tAttr1_DIST	= PO_tAttr1.h PO_tAttr1.tcc PO_tAttr1.sql \
			  tAttr1.run tAttr1.stdout 

tQuery_SOURCES		= tQuery.cc
tQuery_LDADD		= ../src/libpl.la
tQuery_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)
EXTRA_tQuery_DIST	= tQuery.stdout

TESTS			= $(check_SCRIPTS)

XFAIL_TESTS		= tAttr1.sh

TESTS_ENVIRONMENT	= lofar_sharedir=$(lofar_sharedir) \
			  srcdir=$(srcdir) \
			  CHECKTOOL="$(CHECKTOOL)"

EXTRA_DIST		= $(check_SCRIPTS) \
			  $(EXTRA_tA_DIST) \
			  $(EXTRA_tC_DIST) \
			  $(EXTRA_tExpr_DIST) \
			  $(EXTRA_tAttrib_DIST) \
			  $(EXTRA_tCollection_DIST) \
			  $(EXTRA_tObjectId_DIST) \
			  $(EXTRA_tAttr1_DIST) \
			  $(EXTRA_tQuery_DIST) \
			  VersionNrTrigger.plpgsql

include $(top_srcdir)/Makefile.common
