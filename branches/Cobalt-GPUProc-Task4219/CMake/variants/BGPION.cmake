# Definitions file for the BlueGene/P I/O Node cross-compiler
#
# $Id$

# Define compiler suites.
set(LOFAR_COMPILER_SUITES BGPION)

# Define build variants.
set(LOFAR_BUILD_VARIANTS DEBUG OPT)

# Options
option(BUILD_STATIC_EXECUTABLES "Build static executables" OFF)
option(BUILD_SHARED_LIBS        "Build shared libraries"   OFF)
option(USE_BACKTRACE            "Use backtrace"            ON)
option(USE_BOOST_REGEX          "Use Boost regex"          OFF)
option(USE_LOG4CPLUS            "Use log4cplus"            OFF)
option(USE_READLINE             "Use readline"             OFF)
option(USE_SHMEM                "Use shmem"                OFF)
option(USE_THREADS              "Use threads"              ON)
option(USE_VALGRIND             "Use valgrind"             OFF)

# GNU BG/P compiler suite
set(BGPION_COMPILERS BGPION_C BGPION_CXX BGPION_ASM)
set(BGPION_C         /usr/bin/gcc)
set(BGPION_CXX       /usr/bin/g++)
set(BGPION_ASM       /bgsys/drivers/ppcfloor/gnu-linux/bin/powerpc-bgp-linux-g++)

set(BGPION_C_FLAGS)
set(BGPION_C_FLAGS_DEBUG "-g")
set(BGPION_C_FLAGS_OPT "-g -O2")
set(BGPION_CXX_FLAGS "-W -Wall -Woverloaded-virtual -Wno-unknown-pragmas")
set(BGPION_CXX_FLAGS_DEBUG "-g")
set(BGPION_CXX_FLAGS_OPT "-g -O2")
set(BGPION_EXE_LINKER_FLAGS)
set(BGPION_EXE_LINKER_FLAGS_DEBUG)
set(BGPION_EXE_LINKER_FLAGS_OPT)
set(BGPION_COMPILE_DEFINITIONS
  -B/bgsys/drivers/ppcfloor/gnu-linux/powerpc-bgp-linux/bin
  -DHAVE_BGP
  -DHAVE_BGP_ION
  -DHAVE_FCNP

  # disable compiler warnings in the following directories
  -isystem /bgsys/LOFAR/openmpi-1.4.1-ion/include
  -isystem /globalhome/lofarsystem/packages/root/bgp_ion/boost/include

  # include these AFTER mpi, since the directories below also contain mpi.h,
  # but for BGPCN
  -isystem /bgsys/drivers/ppcfloor/comm/include
  -isystem /bgsys/drivers/ppcfloor/arch/include
  )
set(BGPION_COMPILE_DEFINITIONS_DEBUG
  -DENABLE_DBGASSERT
  -DENABLE_TRACER
  -DLOFAR_DEBUG)

# Root directory of the MPI compiler
set(MPI_ROOT_DIR /bgsys/LOFAR/openmpi-ion)

