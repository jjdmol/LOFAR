
## -----------------------------------------------------------------------------
## $Id:: CMakeLists.txt 1475 2008-04-23 11:49:15Z baehren                      $
## -----------------------------------------------------------------------------

## Variables used through the configuration environment:
##
##  LOFAR_ROOT              -- Root of the LOFAR directory tree.
##  LOFAR_CMAKE_CONFIG      -- 
##  LOFAR_LIB_LOCATIONS     -- 
##  LOFAR_INCLUDE_LOCATIONS -- 
##  LOFAR_INSTALL_PREFIX    -- Prefix marking the location at which the finished
##                           software components will be installed
##  LOFAR_VARIANTS_FILE     -- Variants file containing host-specific overrides
##                           to the common configuration settings/presets.
##

## --------------------------------------------------------------------------
## Several "Auto-tools variables" needed for backward compatibility
## --------------------------------------------------------------------------
set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "srcdir")
message (STATUS "srcdir                 = ${srcdir}")

if (NOT LOFAR_CMAKE_CONFIG)

  ## First pass: if LOFAR_ROOT is still undefined we need to define it, since this
  ##             is the common starting point for all directory references below

  if (NOT LOFAR_ROOT)
    message (STATUS "[LOFAR CMake] LOFAR_ROOT undefined; trying to locate it...")
    ## try to find the root directory based on the location of the release
    ## directory
    find_path (LOFAR_INSTALL_PREFIX release/release_area.txt
      $ENV{LOFARSOFT}
      ..
      ../..
      NO_DEFAULT_PATH
      )
    ## convert the relative path to an absolute one
    get_filename_component (LOFAR_ROOT ${LOFAR_INSTALL_PREFIX} ABSOLUTE)
  endif (NOT LOFAR_ROOT)

  ## Second pass: check once more if LOFAR_ROOT is defined
  
  if (LOFAR_ROOT)
    ## This addition to the module path needs to go into the cache,
    ## because otherwise it will be gone at the next time CMake is run
    set (CMAKE_MODULE_PATH ${LOFAR_ROOT}/cmake/modules
      CACHE
      PATH 
      "LOFAR cmake modules"
      FORCE)
    ## installation location
    set (LOFAR_INSTALL_PREFIX ${LOFAR_ROOT}/release
      CACHE
      PATH
      "LOFAR default install area"
      FORCE
      )
    set (CMAKE_INSTALL_PREFIX ${LOFAR_ROOT}/release
      CACHE
      PATH
      "CMake installation area"
      FORCE
      )
    ## header files
    include_directories (${LOFAR_ROOT}/release/include
      CACHE
      PATH
      "LOFAR include area"
      FORCE
      )
      
    ## --------------------------------------------------------------------------
    ## Several "Auto-tools variables" needed for backward compatibility
    ## --------------------------------------------------------------------------
    set(lofar_top_srcdir "${LOFAR_ROOT}" CACHE INTERNAL "lofar_top_srcdir")
    set(lofar_sharedir "${lofar_top_srcdir}/autoconf_share" CACHE INTERNAL "lofar_sharedir")
    set(prefix "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL "prefix")
    
    ## Feedback 
    message (STATUS "[LOFAR CMake configuration]")
    message (STATUS "LOFAR_ROOT             = ${LOFAR_ROOT}")
    message (STATUS "LOFAR_INSTALL_PREFIX   = ${LOFAR_INSTALL_PREFIX}")
    message (STATUS "CMAKE_INSTALL_PREFIX   = ${CMAKE_INSTALL_PREFIX}")
    message (STATUS "lofar_top_srcdir       = ${lofar_top_srcdir}")
    message (STATUS "lofar_sharedir         = ${lofar_sharedir}")
    message (STATUS "prefix                 = ${prefix}")
     
  else (LOFAR_ROOT)
    message (SEND_ERROR "LOFAR_ROOT is undefined!")
  endif (LOFAR_ROOT)
  
  ## ---------------------------------------------------------------------------
  ## locations in which to look for applications/binaries
  
  set (bin_locations
    ${LOFAR_INSTALL_PREFIX}/bin
    /usr/bin
    /usr/local/bin
    /sw/bin
    CACHE
    PATH
    "Extra directories to look for executable files"
    FORCE
    )
  
  ## ----------------------------------------------------------------------------
  ## locations in which to look for header files
  
  set (include_locations
    ${LOFAR_INSTALL_PREFIX}/include
    /opt/include
    /opt/local/include
    /sw/include
    /usr/include
    /usr/local/include
    /usr/X11R6/include
    /opt/aips++/local/include
    /opt/casa/local/include    
    CACHE
    PATH
    "Directories to look for include files"
    FORCE
    )
  
  ## ----------------------------------------------------------------------------
  ## locations in which to look for libraries
  
  set (lib_locations
    ${LOFAR_INSTALL_PREFIX}/lib
    /opt/lib
    /opt/local/lib
    /sw/lib
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /usr/local/lib64
    /usr/X11R6/lib
    /opt/aips++/local/lib
    /Developer/SDKs/MacOSX10.4u.sdk/usr/lib
    CACHE
    PATH
    "Directories to look for libraries"
    FORCE
    )
 
  ## ----------------------------------------------------------------------------
  ## Configuration flag
  
  set (LOFAR_CMAKE_CONFIG TRUE CACHE BOOL "LOFAR CMake configuration flag" FORCE)
  mark_as_advanced(LOFAR_CMAKE_CONFIG)

endif (NOT LOFAR_CMAKE_CONFIG)
