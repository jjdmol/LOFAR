#
# Makefile for the examples that accompany the Doxygen Quick Guide.
#

DOXYGEN=`which doxygen`

doc: group/html/index.html \
     memgrp/html/index.html

clean:
	$(RM) -r group memgrp

group/html/index.html: group.h group.cfg
	@$(DOXYGEN) group.cfg

memgrp/html/index.html: memgrp.h memgrp.cfg
	@$(DOXYGEN) memgrp.cfg

