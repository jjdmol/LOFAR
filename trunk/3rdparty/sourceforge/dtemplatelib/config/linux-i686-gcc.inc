# Written for GNU -*- makefile -*-.

include ../config/gcc.inc
MAKEFILEDEPS += ../config/gcc.inc

# -frepo compiles much faster but can have link problems.  See the info
# page for GCC for details.  This option is not (yet) usable on Cygwin.
CC += -march=i686

# For Linux assume we are using a case sensitive MySQL database 
# - force all queries to be uppper case
# Also gcc 2.96 with MySQL does not seem to support bulk fetch operations with std::string
# correctly so disable these for now
CFLAGS += -DDTL_UC -DDTL_SINGLE_FETCH 

# For some of our internal projects we need to manually define UNIX
# Also, we need to tell unixODBC that gcc supports the "long long" data type
CFLAGS += -DUNIX -DHAVE_LONG_LONG

include ../config/unixODBC.inc
MAKEFILEDEPS += ../config/unixODBC.inc
