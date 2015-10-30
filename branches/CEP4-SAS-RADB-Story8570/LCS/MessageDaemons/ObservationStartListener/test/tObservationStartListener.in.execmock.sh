#!/bin/sh
# Mock program as a value for --exec parameter in tests.
#
# $Id$

# stderr is hooked to the program's log file, which is used for output verif
# looking for (part of) this echo.
echo "[`basename -- $0`] Args: $*" >&2
