bin_PROGRAMS   := uvplot mstouvp


#mstopvd_SOURCES := mstopvd.cc
#mstopvd_DEPENDENCIES := $(top_builddir)/src/libuvplot.la $(LOFAR_DEPEND)
#mstopvd_LDADD := $(top_builddir)/src/libuvplot.la

#kuvplot_SOURCES      := kuvplot_main.cc UVPKarmaViewer.cc
#kuvplot_DEPENDENCIES := $(top_builddir)/src/libuvplot.la $(LOFAR_DEPEND)
#kuvplot_LDADD        := -L/usr/X11R6/lib -L$(KARMABASE)/lib -lkarmawidgets -lkarmagraphics -lkarma -lkarmaXt -lkarmaX11 -lXt -lX11 -lXaw -lpthread $(top_builddir)/src/libuvplot.la



uvplot_SOURCES := main.cc \
                  UVPMainWindow.cc   UVPMainWindow.moc.cc  \
                  UVPDisplayArea.cc  UVPDisplayArea.moc.cc \
                  UVPGraphSettingsWidget.cc UVPGraphSettingsWidget.moc.cc \
                  UVPTimeFrequencyPlot.cc UVPTimeFrequencyPlot.moc.cc \
                  UVPUVCoverageArea.cc UVPUVCoverageArea.moc.cc\
                  UVPOpenVDMDialog.cc\
                  AID-uvplot-Registry.cc


uvplot_LDADD        := $(top_builddir)/src/libuvplot.la
uvplot_DEPENDENCIES := $(top_builddir)/src/libuvplot.la $(LOFAR_DEPEND)

# Your package's subdirectory
#UVPLOT_DIR := $(top_srcdir)/src



#mstouvp_SOURCES := mstouvp.cc
#mstouvp_DEPENDENCIES := $(top_builddir)/src/libuvplot.la $(LOFAR_DEPEND)
#mstouvp_LDADD := $(top_builddir)/src/libuvplot.la


# Your package's subdirectory
#mstopvd_DIR := $(top_srcdir)/test

CXXFLAGS += -DDEBUG_MODE=1 -DHAVE_VDM=1 -I$(KARMABASE)/include



include $(lofar_sharedir)/Makefile.common
