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
                        AID-uvplot-Registry.cc



# Your package's subdirectory
UVPLOT_DIR := $(top_srcdir)/src


CXXFLAGS += -DDEBUG_MODE=1



include $(lofar_sharedir)/Makefile.common
