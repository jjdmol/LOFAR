#
# Makefile for the Doxygen Quick Guide.
#

# Set variables if they are undefined
export LOFAR   ?= $(shell pwd | sed 's|/LOFAR/.*|/LOFAR|')
export DOXYGEN ?= /usr/bin/doxygen
export DOCDIR  ?= $(LOFAR)/doc/html

# Add this documentation to the "doxygen" subdirectory.
THISDOCDIR := $(DOCDIR)/doxygen

doc:
	@if ! [ -d $(THISDOCDIR) ]; then mkdir $(THISDOCDIR); fi
	@cp quick-guide.cfg doxygen.cfg
	@echo "HTML_OUTPUT = $(THISDOCDIR)" >> doxygen.cfg
	@echo "GENERATE_TAGFILE = $(DOCDIR)/doxygen.tag" >> doxygen.cfg
	$(DOXYGEN) doxygen.cfg
	$(MAKE) doc -C examples

clean:
	$(RM) -r $(THISDOCDIR) $(DOCDIR)/doxygen.tag doxygen.cfg
	$(MAKE) clean -C examples

