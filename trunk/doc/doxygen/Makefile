#
# Makefile for the Doxygen Quick Guide.
#

# Export all variables
export 

# Set variables if they are undefined
DOXYGEN ?= /usr/bin/doxygen
DOCDIR  ?= ../html

# Add this documentation to the "doxygen" subdirectory.
DOCDIR  := $(DOCDIR)/doxygen

doc:
	@if ! [ -d $(DOCDIR) ]; then mkdir $(DOCDIR); fi
	@cp quick-guide.cfg doxygen.cfg
	@echo "HTML_OUTPUT = $(DOCDIR)" >> doxygen.cfg
	@echo "GENERATE_TAGFILE = $(DOCDIR)/../doxygen.tag" >> doxygen.cfg
	$(DOXYGEN) doxygen.cfg
	$(MAKE) doc -C examples

clean:
	$(RM) -r $(DOCDIR)
	@$(MAKE) clean -C examples
