AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

check_PROGRAMS 		= tmEcho tmPing tGCFPort tTimer \
						tGCFTask1 tGCFTask2 tGCFTask3 tGCFTask4 tGCFTask5

tmEcho_SOURCES 		= $(BUILT_SOURCES) Echo_Protocol.cc Echo.cc Echo.h 
tmEcho_LDADD		= ../src/libgcftm.la
tmEcho_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tmPing_SOURCES 		= $(BUILT_SOURCES) Echo_Protocol.cc Ping.cc Ping.h
tmPing_LDADD		= ../src/libgcftm.la
tmPing_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFPort_SOURCES 		= $(BUILT_SOURCES) Echo_Protocol.cc \
						  tGCFPort.cc tGCFPort.h 
tGCFPort_LDADD			= ../src/libgcftm.la
tGCFPort_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tTimer_SOURCES 		= $(BUILT_SOURCES) tTimer.cc
tTimer_LDADD		= ../src/libgcftm.la
tTimer_DEPENDENCIES	= ../src/libgcftm.la tTimer.h $(LOFAR_DEPEND)

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



