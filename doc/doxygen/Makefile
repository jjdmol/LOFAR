#
# Makefile for the Doxygen Quick Guide.
#

DOXYGEN=`which doxygen`

doc:
	@$(DOXYGEN) quick-guide.cfg
	@$(MAKE) doc -C examples

clean:
	$(RM) -r html
	@$(MAKE) clean -C examples
