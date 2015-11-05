# Definitions file for the CLANG compiler suite.
#
# $Id$


# Compiler suite
set(LOFAR_COMPILER_SUITES CLANG)

# Build variants
set(LOFAR_BUILD_VARIANTS DEBUG OPT OPT3)

# CLANG compiler suite
set(CLANG_COMPILERS CLANG_C CLANG_CXX CLANG_Fortran CLANG_ASM)
set(CLANG_C         /usr/bin/clang)    # CLANG C compiler
set(CLANG_CXX       /usr/bin/clang++)  # CLANG C++ compiler
set(CLANG_Fortran   /usr/bin/gfortran) # CLANG Fortran compiler
set(CLANG_ASM       /usr/bin/gcc)      # CLANG assembler

set(CLANG_C_FLAGS          "-W -Wall -Wno-unknown-pragmas")
set(CLANG_C_FLAGS_DEBUG    "-g")
set(CLANG_C_FLAGS_OPT      "-g -O2")
set(CLANG_C_FLAGS_OPT3     "-g -O3")
set(CLANG_CXX_FLAGS        "-W -Wall -Woverloaded-virtual -Wno-unknown-pragmas")
set(CLANG_CXX_FLAGS_DEBUG  "-g")
set(CLANG_CXX_FLAGS_OPT    "-g -O2")
set(CLANG_CXX_FLAGS_OPT3   "-g -O3")
set(CLANG_EXE_LINKER_FLAGS)
set(CLANG_EXE_LINKER_FLAGS_DEBUG)
set(CLANG_EXE_LINKER_FLAGS_OPT)
set(CLANG_EXE_LINKER_FLAGS_OPT3)
set(CLANG_SHARED_LINKER_FLAGS)
set(CLANG_SHARED_LINKER_FLAGS_DEBUG)
set(CLANG_SHARED_LINKER_FLAGS_OPT)
set(CLANG_SHARED_LINKER_FLAGS_OPT3)
set(CLANG_COMPILE_DEFINITIONS)
set(CLANG_COMPILE_DEFINITIONS_DEBUG 
                           "-DLOFAR_DEBUG -DENABLE_DBGASSERT -DENABLE_TRACER")
set(CLANG_COMPILE_DEFINITIONS_OPT)
set(CLANG_COMPILE_DEFINITIONS_OPT3
                         "-DNDEBUG -DDISABLE_DEBUG_OUTPUT")
