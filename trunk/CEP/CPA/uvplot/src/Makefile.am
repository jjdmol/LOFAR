bin_PROGRAMS   := uvplot
uvplot_SOURCES := main.cc \
                  UVPMainWindow.cc   UVPMainWindow.moc.cc  \
                  UVPDisplayArea.cc  UVPDisplayArea.moc.cc \
                  UVPPixel.cc          \
                  UVPAxis.cc           \
                  UVPImageCube.cc      \
                  UVPSpectrum.cc       \
                  UVPSpectrumVector.cc \
                  UVPTimeFrequencyPlot.cc UVPTimeFrequencyPlot.moc.cc \
                  UVPUVCoverageArea.cc UVPUVCoverageArea.moc.cc \
                  UVPDataTransferWP.cc \
                  UVPDataAtom.cc       \
                  UVPDataHeader.cc     \
                  AID-uvplot-Registry.cc


# Your package's subdirectory
UVPLOT_DIR := $(top_srcdir)/src


CXXFLAGS += -DDEBUG_MODE -pg



include $(lofar_sharedir)/Makefile.common
