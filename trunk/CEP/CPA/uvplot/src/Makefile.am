lib_LTLIBRARIES      := libuvplot.la
libuvplot_la_SOURCES := UVPGraphSettings.cc    \
                        UVPPixel.cc            \
                        UVPAxis.cc             \
                        UVPImageCube.cc        \
                        UVPSpectrum.cc         \
                        UVPSpectrumVector.cc   \
                        UVPDataHeader.cc       \
                        UVPDataAtomHeader.cc   \
                        UVPDataAtom.cc         \
                        UVPDataAtomVector.cc   \
                        UVPDataCacheElement.cc \
                        UVPDataTransferWP.cc   \
	                     UVPMessagesToFileWP.cc \
                        UVPPVDInput.cc         \
                        AID-uvplot-Registry.cc
#                       UVPCorrelationDataCache.cc \
#                       UVPBaselineDataCache.cc 


bin_PROGRAMS   := uvplot
uvplot_SOURCES := main.cc \
                  UVPMainWindow.cc   UVPMainWindow.moc.cc  \
                  UVPDisplayArea.cc  UVPDisplayArea.moc.cc \
                  UVPGraphSettingsWidget.cc UVPGraphSettingsWidget.moc.cc \
                  UVPTimeFrequencyPlot.cc UVPTimeFrequencyPlot.moc.cc \
                  UVPUVCoverageArea.cc UVPUVCoverageArea.moc.cc


uvplot_LDADD        := $(top_builddir)/src/libuvplot.la
uvplot_DEPENDENCIES := $(top_builddir)/src/libuvplot.la $(LOFAR_DEPEND)

# Your package's subdirectory
UVPLOT_DIR := $(top_srcdir)/src


#LDADD := $(top_builddir)/src/libuvplot.la


CXXFLAGS += -DDEBUG_MODE=1



include $(lofar_sharedir)/Makefile.common
