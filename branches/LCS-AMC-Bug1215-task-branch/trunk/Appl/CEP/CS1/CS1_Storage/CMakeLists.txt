
## ------------------------------------------------------------------------------
## Project characteristics

project (CS1_Storage)

cmake_minimum_required (VERSION 2.5)

## ------------------------------------------------------------------------------
## Dependency on other packages

if (HAVE_APS)	
  add_definitions (-DHAVE_APS)
endif (HAVE_APS)

if (HAVE_TINYCEP)	
  add_definitions (-DHAVE_TINYCEP)
endif (HAVE_TINYCEP)

## ------------------------------------------------------------------------------
## Directories to be included in the build

add_subdirectory (include)
add_subdirectory (src)
##add_subdirectory (test)


## ------------------------------------------------------------------------------
## Summary
