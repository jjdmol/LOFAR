# Written for GNU -*- makefile -*-.

include ../config/gcc.inc
MAKEFILEDEPS += ../config/gcc.inc

# -frepo compiles much faster but can have link problems.  See the info
# page for GCC for details.  This option is not (yet) usable on Cygwin.
CC += -mtune=itanium2

# For Linux assume we are using a case sensitive MySQL database 
# - force all queries to be uppper case
# Assume bulk fetch is supported, this requires MyODBC 3.51 or higher.
CFLAGS += -DDTL_UC  

# For some of our internal projects we need to manually define UNIX
# Also, we need to tell unixODBC that gcc supports the "long long" data type
CFLAGS += -DUNIX -DHAVE_LONG_LONG

# ASTRON
# Setup compilerflags to compile our own patches.
CFLAGS += -DPOSTGRES_BLOB_PATCH

# ASTRON
# On Linux, libdl.so should be linked in as well.
LDFLAGS += -ldl

# ASTRON, lofar17
# For some reason the ODBC libraries in /usr/lib are broken on lofar17.
# Hence, we need to add /usr/local/lib to the library search path.
LDFLAGS += -L/usr/local/lib

include ../config/unixODBC.inc
MAKEFILEDEPS += ../config/unixODBC.inc
