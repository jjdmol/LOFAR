# Definitions file for the GNU compiler suite.
#
# $Id: GNU.cmake 26820 2013-10-02 13:09:54Z froehlich $


# Compiler suite
set(LOFAR_COMPILER_SUITES GNU_JUROPA)

# Build variants
set(LOFAR_BUILD_VARIANTS DEBUG OPT)

# GNU compiler suite
set(GNU_JUROPA_COMPILERS GNU_C GNU_CXX GNU_Fortran GNU_ASM)
set(GNU_C         /usr/local/gcc/gcc-4.6.3/bin/gcc)      # GNU C compiler
set(GNU_CXX       /usr/local/gcc/gcc-4.6.3/bin/g++)      # GNU C++ compiler
set(GNU_Fortran   /usr/local/gcc/gcc-4.6.3/bin/gfortran) # GNU Fortran compiler
set(GNU_ASM       /usr/local/gcc/gcc-4.6.3/bin/gcc)      # GNU assembler

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
set(GNU_COMPILE_DEFINITIONS_DEBUG 
                         "-DLOFAR_DEBUG -DENABLE_DBGASSERT -DENABLE_TRACER")
set(GNU_COMPILE_DEFINITIONS_OPT)
