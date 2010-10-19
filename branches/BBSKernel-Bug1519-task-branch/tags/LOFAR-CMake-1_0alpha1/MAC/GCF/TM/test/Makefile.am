AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

check_PROGRAMS 			= tAutoOpen tEventQueue tStateResult tmEcho tmPing tGCFPort tGCFTimer \
						tGCFTask1 tGCFTask2 tGCFTask3 tGCFTask4 tGCFTask5

tmEcho_SOURCES 			= $(BUILT_SOURCES) Echo.cc Echo_Protocol.cc
tmEcho_LDADD			= ../src/libgcftm.la
tmEcho_DEPENDENCIES		= ../src/libgcftm.la $(LOFAR_DEPEND)

tmPing_SOURCES 			= $(BUILT_SOURCES) Ping.cc Echo_Protocol.cc
tmPing_LDADD			= ../src/libgcftm.la
tmPing_DEPENDENCIES		= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFPort_SOURCES 		= $(BUILT_SOURCES) Echo_Protocol.cc \
						  tGCFPort.cc tGCFPort.h 
tGCFPort_LDADD			= ../src/libgcftm.la
tGCFPort_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFTimer_SOURCES 		= $(BUILT_SOURCES) tTimer.cc
tGCFTimer_LDADD			= ../src/libgcftm.la
tGCFTimer_DEPENDENCIES		= ../src/libgcftm.la tTimer.h $(LOFAR_DEPEND)

tEventQueue_SOURCES 	= $(BUILT_SOURCES) tEventQueue.cc tClient.cc Echo_Protocol.cc
tEventQueue_LDADD		= ../src/libgcftm.la
tEventQueue_DEPENDENCIES= ../src/libgcftm.la $(LOFAR_DEPEND)

tAutoOpen_SOURCES 		= $(BUILT_SOURCES) tAutoOpen.cc tServer.cc Echo_Protocol.cc
tAutoOpen_LDADD			= ../src/libgcftm.la
tAutoOpen_DEPENDENCIES  = ../src/libgcftm.la $(LOFAR_DEPEND)

tStateResult_SOURCES	 = $(BUILT_SOURCES) tStateResult.cc tServer.cc Echo_Protocol.cc
tStateResult_LDADD		 = ../src/libgcftm.la
tStateResult_DEPENDENCIES= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFTask1_SOURCES 		= $(BUILT_SOURCES) tGCFTask1.cc testTask.cc
tGCFTask1_LDADD			= ../src/libgcftm.la
tGCFTask1_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFTask2_SOURCES 		= $(BUILT_SOURCES) tGCFTask2.cc testTask.cc
tGCFTask2_LDADD			= ../src/libgcftm.la
tGCFTask2_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFTask3_SOURCES 		= $(BUILT_SOURCES) tGCFTask3.cc testTask.cc
tGCFTask3_LDADD			= ../src/libgcftm.la
tGCFTask3_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFTask4_SOURCES 		= $(BUILT_SOURCES) tGCFTask4.cc testTask.cc
tGCFTask4_LDADD			= ../src/libgcftm.la
tGCFTask4_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFTask5_SOURCES 		= $(BUILT_SOURCES) tGCFTask5.cc testTask.cc
tGCFTask5_LDADD			= ../src/libgcftm.la
tGCFTask5_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

BUILT_SOURCES = Echo_Protocol.ph 

EXTRA_DIST = \
	tmEcho.conf \
	tmPing.conf \
	tGCFPort.conf
	
%.conf:	tm-test.conf.in
	ln -sf $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common



