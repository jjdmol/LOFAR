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
                  UVPUVCoverageArea.cc UVPUVCoverageArea.moc.cc


# Your package's subdirectory
UVPLOT_DIR := $(top_srcdir)/src

#QT_DIR     := /home/brentjens/usr/src/qt-2.3.2
#MOC        := $(QT_DIR)/bin/moc

#%.moc.cc: %.h
#	$(MOC) $< -o $@

CXXFLAGS += -pg #-I$(QT_DIR)/include -L$(QT_DIR)/lib -lqt


include $(lofar_sharedir)/Makefile.common
