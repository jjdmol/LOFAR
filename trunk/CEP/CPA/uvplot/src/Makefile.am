bin_PROGRAMS   := uvplot
uvplot_SOURCES := main.cc \
                  UVPMainWindow.cc   UVPMainWindow.moc.cc  \
                  UVPDisplayArea.cc  UVPDisplayArea.moc.cc \
                  UVPGraphSettings.cc    \
                  UVPGraphSettingsWidget.cc UVPGraphSettingsWidget.moc.cc \
                  UVPPixel.cc            \
                  UVPAxis.cc             \
                  UVPImageCube.cc        \
                  UVPSpectrum.cc         \
                  UVPSpectrumVector.cc   \
                  UVPTimeFrequencyPlot.cc UVPTimeFrequencyPlot.moc.cc \
                  UVPUVCoverageArea.cc UVPUVCoverageArea.moc.cc \
                  UVPDataTransferWP.cc   \
                  UVPDataHeader.cc       \
                  UVPDataAtomHeader.cc   \
                  UVPDataAtom.cc         \
                  UVPDataCacheElement.cc \
#                  UVPCorrelationDataCache.cc \
#                  UVPBaselineDataCache.cc    \
                  AID-uvplot-Registry.cc


# Your package's subdirectory
UVPLOT_DIR := $(top_srcdir)/src


CXXFLAGS += -DDEBUG_MODE=1



include $(lofar_sharedir)/Makefile.common
