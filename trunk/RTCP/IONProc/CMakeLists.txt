
## ------------------------------------------------------------------------------
## Project characteristics

project (CS1_IONProc)

cmake_minimum_required (VERSION 2.5)

## ------------------------------------------------------------------------------
## Dependency on other packages

if (HAVE_APS)	
  add_definitions (-DHAVE_APS)
endif (HAVE_APS)

if (HAVE_TINYCEP)	
  add_definitions (-DHAVE_TINYCEP)
endif (HAVE_TINYCEP)

add_subdirectory (src)
