check_PROGRAMS		= \
			  tExpr \
			  tA \
			  tC \
			  tAttrib \
			  tCollection \
			  tObjectId \
			  tAttr1 \
			  tQuery

# programs to run through a checktool
CHECKTOOLPROGS           = \
			  tExpr \
			  tAttrib \
			  tCollection \
			  tObjectId \
			  tAttr1 \
			  tQuery
#ENDCHECKTOOLPROGS

tA_SOURCES              = tA.cc
tA_LDADD		= ../src/libpl.la
tA_DEPENDENCIES		= ../src/libpl.la $(LOFAR_DEPEND)

tC_SOURCES              = tC.cc
tC_LDADD		= ../src/libpl.la
tC_DEPENDENCIES		= ../src/libpl.la $(LOFAR_DEPEND)

tExpr_SOURCES		= tExpr.cc
tExpr_LDADD		= ../src/libpl.la
tExpr_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tAttrib_SOURCES		= tAttrib.cc
tAttrib_LDADD		= ../src/libpl.la
tAttrib_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tCollection_SOURCES	= tCollection.cc
tCollection_LDADD	= ../src/libpl.la
tCollection_DEPENDENCIES= ../src/libpl.la $(LOFAR_DEPEND)

tObjectId_SOURCES	= tObjectId.cc
tObjectId_LDADD		= ../src/libpl.la
tObjectId_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tAttr1_SOURCES		= tAttr1.cc
tAttr1_LDADD		= ../src/libpl.la
tAttr1_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

tQuery_SOURCES		= tQuery.cc
tQuery_LDADD		= ../src/libpl.la
tQuery_DEPENDENCIES	= ../src/libpl.la $(LOFAR_DEPEND)

TESTS			= \
			  tA_test.sh \
			  tC_test.sh \
			  tExpr.sh \
			  tAttrib.sh \
			  tObjectId.sh \
			  tCollection.sh \
			  tAttr1.sh \
			  tQuery.sh

TESTS_ENVIRONMENT	= lofar_sharedir=$(lofar_sharedir) \
			  CHECKTOOL="$(CHECKTOOL)"


include $(lofar_sharedir)/Makefile.common
