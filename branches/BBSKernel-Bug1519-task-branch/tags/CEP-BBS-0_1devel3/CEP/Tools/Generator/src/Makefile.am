
lib_LTLIBRARIES         = libgenerator.la

libgenerator_la_SOURCES  = Flaggers.cc \
		 	Signals.cc \
				RSPTimeStamp.cc \
		   	        WH_FakeStation.cc \
				WH_Signal.cc \
				AH_Recorder.cc \
				AH_Generator.cc \
				DH_RSP.cc \
				Connector.cc

bin_PROGRAMS = Recorder Generator

# Lines to build a program prg
Recorder_SOURCES	= Recorder_main.cc
Recorder_LDADD		= libgenerator.la
Recorder_DEPENDENCIES	= libgenerator.la $(LOFAR_DEPEND)
Generator_SOURCES	= Generator_main.cc
Generator_LDADD		= libgenerator.la
Generator_DEPENDENCIES	= libgenerator.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = 

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(top_srcdir)/Makefile.common
