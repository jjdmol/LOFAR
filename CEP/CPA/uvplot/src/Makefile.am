bin_PROGRAMS   := uvplot
uvplot_SOURCES := main.cc mainwin.cc mainwin.moc.cc \
                  UVPDisplayArea.cc \
                  UVPPixel.cc     \
                  UVPAxis.cc      \
                  UVPImageCube.cc


# Your package's subdirectory
UVPLOT_DIR := $(top_srcdir)/src

QT_DIR     := /home/brentjens/usr/src/qt-2.3.2
MOC        := $(QT_DIR)/bin/moc

%.moc.cc: %.h
	$(MOC) $< -o $@

CXXFLAGS += -Wall -I$(QT_DIR)/include -L$(QT_DIR)/lib -lqt


include $(lofar_sharedir)/Makefile.common
