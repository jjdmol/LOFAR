bin_PROGRAMS   := mstouvp
#mstopvd_SOURCES := mstopvd.cc
#mstopvd_DEPENDENCIES := $(top_builddir)/src/libuvplot.la $(LOFAR_DEPEND)
#mstopvd_LDADD := $(top_builddir)/src/libuvplot.la

#kuvplot_SOURCES      := kuvplot_main.cc UVPKarmaViewer.cc
#kuvplot_DEPENDENCIES := $(top_builddir)/src/libuvplot.la $(LOFAR_DEPEND)
#kuvplot_LDADD        := -L/usr/X11R6/lib -L$(KARMABASE)/lib -lkarmawidgets -lkarmagraphics -lkarma -lkarmaXt -lkarmaX11 -lXt -lX11 -lXaw -lpthread $(top_builddir)/src/libuvplot.la

mstouvp_SOURCES := mstouvp.cc
mstouvp_DEPENDENCIES := $(top_builddir)/src/libuvplot.la $(LOFAR_DEPEND)
mstouvp_LDADD := $(top_builddir)/src/libuvplot.la


# Your package's subdirectory
#mstopvd_DIR := $(top_srcdir)/test

CXXFLAGS += -g -DDEBUG_MODE=1 -I$(KARMABASE)/include



include $(lofar_sharedir)/Makefile.common
