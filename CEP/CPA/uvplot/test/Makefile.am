bin_PROGRAMS   := mstopvd kuvplot
mstopvd_SOURCES := mstopvd.cc
mstopvd_DEPENDENCIES :=  $(LOFAR_DEPEND)

kuvplot_SOURCES      := kuvplot_main.cc UVPKarmaViewer.cc
kuvplot_DEPENDENCIES := $(LOFAR_DEPEND)
kuvplot_LDADD        := -L/usr/X11R6/lib -L$(KARMABASE)/lib -lkarmawidgets -lkarmagraphics -lkarma -lkarmaXt -lkarmaX11 -lXt -lX11 -lXaw -lpthread

# Your package's subdirectory
mstopvd_DIR := $(top_srcdir)/test
kuvplot_DIR := $(top_srcdir)/test

CXXFLAGS += -g -DDEBUG_MODE=1 -I$(KARMABASE)/include



include $(lofar_sharedir)/Makefile.common
