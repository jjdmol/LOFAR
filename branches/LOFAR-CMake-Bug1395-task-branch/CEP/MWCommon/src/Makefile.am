lib_LTLIBRARIES		= libmwcommon.la

libmwcommon_la_SOURCES	= Package__Version.cc \
ClusterDesc.cc \
ControllerBase.cc \
DomainShape.cc \
MPIConnection.cc \
MPIConnectionSet.cc \
MWBlobIO.cc \
MWConnection.cc \
MWConnectionSet.cc \
MWGlobalStep.cc \
MWIos.cc \
MWLocalStep.cc \
MWMultiStep.cc \
MWStep.cc \
MWStepFactory.cc \
MWStepVisitor.cc \
MasterControl.cc \
MemConnection.cc \
MemConnectionSet.cc \
NodeDesc.cc \
ObsDomain.cc \
ParameterHandler.cc \
SocketConnection.cc \
SocketConnectionSet.cc \
SocketListener.cc \
VdsDesc.cc \
VdsPartDesc.cc \
WorkDomainSpec.cc \
WorkerControl.cc \
WorkerFactory.cc \
WorkerInfo.cc \
WorkerProxy.cc \
WorkersDesc.cc

bin_PROGRAMS 		= versionmwcommon finddproc showclusterdesc

versionmwcommon_SOURCES      = versionmwcommon.cc
versionmwcommon_LDADD        = libmwcommon.la
versionmwcommon_DEPENDENCIES = libmwcommon.la $(LOFAR_DEPEND)

finddproc_SOURCES      = finddproc.cc
finddproc_LDADD        = libmwcommon.la
finddproc_DEPENDENCIES = libmwcommon.la $(LOFAR_DEPEND)

showclusterdesc_SOURCES      = showclusterdesc.cc
showclusterdesc_LDADD        = libmwcommon.la
showclusterdesc_DEPENDENCIES = libmwcommon.la $(LOFAR_DEPEND)

scriptdir = $(bindir)
dist_script_SCRIPTS = startdistproc socketrun runlogpid

include $(top_srcdir)/Makefile.common
