
## ------------------------------------------------------------------------------
## Process input files for Flex and Bison

if (yacc_bin)
  execute_process (
    WORKING_DIRECTORY ${CEP_SOURCE_DIR}/tinyCEP/src
    COMMAND yacc -d -p SimulatorParse -o SimulatorParse.h SimulatorParse.yy
    TIMEOUT 20
    ERROR_VARIABLE yacc_error
    OUTPUT_QUIET
    )
else (yacc_bin)
  message (SEND_ERROR "Unable to run YACC; missing application!")
endif (yacc_bin)

if (flex_bin)
   execute_process (
    WORKING_DIRECTORY ${CEP_SOURCE_DIR}/tinyCEP/src
    COMMAND flex -t -PSimulatorTokenize SimulatorTokenize.ll
    TIMEOUT 20
    OUTPUT_FILE SimulatorTokenize.cc
    ERROR_VARIABLE lex_error
    OUTPUT_QUIET
   )
else (flex_bin)
  message (SEND_ERROR "Unable to run FLEX; missing application!")
endif (flex_bin)

## ------------------------------------------------------------------------------
## Build the library

file (GLOB libtinycep_SOURCES *.cc)

add_library (tinycep ${libtinycep_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS tinycep
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

