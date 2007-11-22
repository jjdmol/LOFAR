#
# Makefile for the examples that accompany the Doxygen Quick Guide.
#

# Set variables if they are undefined
export LOFAR   ?= $(shell pwd | sed 's|/LOFAR/.*|/LOFAR|')
export DOXYGEN ?= /usr/bin/doxygen
export DOCDIR  ?= $(LOFAR)/doc/html

# Add this documentation to the "doxygen/examples" subdirectory.
THISDOCDIR := $(DOCDIR)/doxygen/examples

doc: $(THISDOCDIR)/group/index.html \
     $(THISDOCDIR)/memgrp/index.html

clean:
	$(RM) -r $(THISDOCDIR) doxygen.cfg

$(THISDOCDIR)/group/index.html: group.h group.cfg
	@if ! [ -d $(THISDOCDIR) ]; then mkdir $(THISDOCDIR); fi
	@cp group.cfg doxygen.cfg
	@echo "HTML_OUTPUT          = $(THISDOCDIR)/group" >> doxygen.cfg
	$(DOXYGEN) doxygen.cfg

$(THISDOCDIR)/memgrp/index.html: memgrp.h memgrp.cfg
	@if ! [ -d $(THISDOCDIR) ]; then mkdir $(THISDOCDIR); fi
	@cp memgrp.cfg doxygen.cfg
	@echo "HTML_OUTPUT          = $(THISDOCDIR)/memgrp" >> doxygen.cfg
	$(DOXYGEN) doxygen.cfg
