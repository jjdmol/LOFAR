#
# Makefile for the examples that accompany the Doxygen Quick Guide.
#

DOXYGEN=`which doxygen`

# Determine the "root" directory of the LOFAR tree.
LOFAR=`pwd | sed 's|/LOFAR/.*|/LOFAR|'`
export LOFAR

doc: group/html/index.html \
     memgrp/html/index.html

clean:
	$(RM) -r group memgrp

group/html/index.html: group.h group.cfg
	@$(DOXYGEN) group.cfg

memgrp/html/index.html: memgrp.h memgrp.cfg
	@$(DOXYGEN) memgrp.cfg

