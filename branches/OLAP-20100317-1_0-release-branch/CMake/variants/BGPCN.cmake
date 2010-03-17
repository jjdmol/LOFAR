# Definitions file for the BlueGene/P Compute Node cross-compiler
#
# $Id$

# Define compiler suites.
set(LOFAR_COMPILER_SUITES BGPCN)

# Define build variants.
set(LOFAR_BUILD_VARIANTS DEBUG OPT)

# Options
option(BUILD_STATIC_EXECUTABLES "Build static executables" ON)
option(USE_BACKTRACE            "Use backtrace"            ON)
option(USE_LOG4CPLUS            "Use log4cplus"            OFF)
option(USE_SHMEM                "Use shmem"                OFF)
option(USE_THREADS              "Use threads"              OFF)
option(USE_VALGRIND             "Use valgrind"             OFF)

# GNU BG/P compiler suite
set(BGPCN_COMPILERS BGPCN_C BGPCN_CXX BGPCN_ASM)
set(BGPCN_C         /usr/bin/gcc)
set(BGPCN_CXX       /usr/bin/g++)
set(BGPCN_ASM       /bgsys/drivers/ppcfloor/gnu-linux/bin/powerpc-bgp-linux-g++)

set(BGPCN_C_FLAGS)
set(BGPCN_C_FLAGS_DEBUG   "-g")
set(BGPCN_C_FLAGS_OPT     "-g -O2")
set(BGPCN_CXX_FLAGS       "-W -Wall -Woverloaded-virtual -Wno-unknown-pragmas")
set(BGPCN_CXX_FLAGS_DEBUG "-g")
set(BGPCN_CXX_FLAGS_OPT   "-g -O2")
set(BGPCN_EXE_LINKER_FLAGS)
set(BGPCN_EXE_LINKER_FLAGS_DEBUG)
set(BGPCN_EXE_LINKER_FLAGS_OPT)
set(BGPCN_COMPILE_DEFINITIONS
  -B/bgsys/drivers/ppcfloor/gnu-linux/powerpc-bgp-linux/bin
  -DHAVE_BGP
  -DHAVE_BGP_CN
  -DHAVE_FCNP
  -DHAVE_MPI
  -I/bgsys/drivers/ppcfloor/comm/include
  -I/bgsys/drivers/ppcfloor/arch/include)
set(BGPCN_COMPILE_DEFINITIONS_DEBUG
  -DENABLE_DBGASSERT
  -DENABLE_TRACER
  -DLOFAR_DEBUG)

# Re-root search to this directory first; needed for cross-compilation
set(CMAKE_FIND_ROOT_PATH /bgsys/drivers/ppcfloor/gnu-linux/powerpc-bgp-linux)
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
