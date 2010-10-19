# Definitions file for the GNU compiler suite.
#
# $Id$


# Compiler suite
set(LOFAR_COMPILER_SUITES GNU)

# Build variants
set(LOFAR_BUILD_VARIANTS DEBUG OPT)

# GNU compiler suite
set(GNU_COMPILERS GNU_C GNU_CXX GNU_ASM)
set(GNU_C         /usr/bin/gcc )  # GNU C compiler
set(GNU_CXX       /usr/bin/g++ )  # GNU C++ compiler
set(GNU_ASM       /usr/bin/gcc )  # GNU assembler

set(GNU_C_FLAGS)
set(GNU_C_FLAGS_DEBUG    "-g")
set(GNU_C_FLAGS_OPT      "-g -O2")
set(GNU_CXX_FLAGS        "-W -Wall -Woverloaded-virtual -Wno-unknown-pragmas")
set(GNU_CXX_FLAGS_DEBUG  "-g")
set(GNU_CXX_FLAGS_OPT    "-g -O2")
set(GNU_EXE_LINKER_FLAGS)
set(GNU_EXE_LINKER_FLAGS_DEBUG)
set(GNU_EXE_LINKER_FLAGS_OPT)
set(GNU_COMPILE_DEFINITIONS)
set(GNU_COMPILE_DEFINITIONS_DEBUG "-DLOFAR_DEBUG -DENABLE_TRACER")
set(GNU_COMPILE_DEFINITIONS_OPT)

