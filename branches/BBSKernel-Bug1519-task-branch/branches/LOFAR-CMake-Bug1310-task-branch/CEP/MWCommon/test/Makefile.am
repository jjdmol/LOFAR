check_PROGRAMS	= tClusterDesc tNodeDesc tVdsDesc tVdsPartDesc tWorkersDesc \
		  tSocketConnection tstartdproc

CHECKTOOLPROGS	= $(CHECK_PROGRAMS)

TESTS		= tClusterDesc.sh tNodeDesc.sh tVdsDesc.sh tVdsPartDesc.sh \
		  tWorkersDesc.sh \
		  tSocketConnection.sh \
		  tfinddproc.sh tstartdproc.sh trundist.sh

LDADD		= ../src/libmwcommon.la 

tClusterDesc_SOURCES 	  = tClusterDesc.cc
tClusterDesc_DEPENDENCIES = ../src/libmwcommon.la $(LOFAR_DEPEND)

tNodeDesc_SOURCES 	  = tNodeDesc.cc
tNodeDesc_DEPENDENCIES	  = ../src/libmwcommon.la $(LOFAR_DEPEND)

tVdsDesc_SOURCES 	  = tVdsDesc.cc
tVdsDesc_DEPENDENCIES	  = ../src/libmwcommon.la $(LOFAR_DEPEND)

tVdsPartDesc_SOURCES 	  = tVdsPartDesc.cc
tVdsPartDesc_DEPENDENCIES = ../src/libmwcommon.la $(LOFAR_DEPEND)

tWorkersDesc_SOURCES 	  = tWorkersDesc.cc
tWorkersDesc_DEPENDENCIES = ../src/libmwcommon.la $(LOFAR_DEPEND)

tSocketConnection_SOURCES 	  = tSocketConnection.cc
tSocketConnection_DEPENDENCIES	  = ../src/libmwcommon.la $(LOFAR_DEPEND)

tstartdproc_SOURCES 	  = tstartdproc.cc

EXTRA_DIST	= tSocketConnection.run tSocketConnection.stdout \
		  tfinddproc.in_vd tfinddproc.in_cd \
		  tfinddproc.run tfinddproc.stdout 
		  tstartdproc.in_vd tstartdproc.in_cd tstartdproc.in_cd1 \
		  tstartdproc.run tstartdproc.in_run tstartdproc.stdout 

include $(top_srcdir)/Makefile.common

