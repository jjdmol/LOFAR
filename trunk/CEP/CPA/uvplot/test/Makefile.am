bin_PROGRAMS   := mstopvd kuvplot mstouvp
mstopvd_SOURCES := mstopvd.cc
mstopvd_DEPENDENCIES :=  $(LOFAR_DEPEND)
mstopvd_LDADD := ../src/libuvplot.la

kuvplot_SOURCES      := kuvplot_main.cc UVPKarmaViewer.cc
kuvplot_DEPENDENCIES := $(LOFAR_DEPEND)
kuvplot_LDADD        := -L/usr/X11R6/lib -L$(KARMABASE)/lib -lkarmawidgets -lkarmagraphics -lkarma -lkarmaXt -lkarmaX11 -lXt -lX11 -lXaw -lpthread ../src/libuvplot.la

mstouvp_SOURCES := mstouvp.cc
mstouvp_DEPENDENCIES :=  $(LOFAR_DEPEND)
mstouvp_LDADD := ../src/libuvplot.la


# Your package's subdirectory
mstopvd_DIR := $(top_srcdir)/test
mstouvp_DIR := $(top_srcdir)/test

CXXFLAGS += -g -DDEBUG_MODE=1 -I$(KARMABASE)/include



include $(lofar_sharedir)/Makefile.common
