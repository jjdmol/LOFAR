#
# Makefile for the examples that accompany the Doxygen Quick Guide.
#

# Export all variables
export

# Set variables if they are undefined
#DOXYGEN ?= /usr/bin/doxygen
#DOCDIR  ?= ../../html

# Add this documentation to the "examples" subdirectory.
DOCDIR  := $(DOCDIR)/examples

# Determine the "root" directory of the LOFAR tree.
LOFAR=`pwd | sed 's|/LOFAR/.*|/LOFAR|'`

doc: $(DOCDIR)/group/index.html \
     $(DOCDIR)/memgrp/index.html

clean:
	$(RM) -r $(DOCDIR)

$(DOCDIR)/group/index.html: group.h group.cfg
	@if ! [ -d $(DOCDIR) ]; then mkdir $(DOCDIR); fi
	@cp group.cfg doxygen.cfg
	@echo "HTML_OUTPUT          = $(DOCDIR)/group" >> doxygen.cfg
	@$(DOXYGEN) doxygen.cfg

$(DOCDIR)/memgrp/index.html: memgrp.h memgrp.cfg
	@if ! [ -d $(DOCDIR) ]; then mkdir $(DOCDIR); fi
	@cp memgrp.cfg doxygen.cfg
	@echo "HTML_OUTPUT          = $(DOCDIR)/memgrp" >> doxygen.cfg
	@$(DOXYGEN) doxygen.cfg

