# - Generic CMake macros for LOFAR
#
# Defines the following macros:
#   force_option(var doc-string [value])
#   join_arguments(var)
#   list_append_if(condition var value1..valuen)
#   lofar_add_bin_program(name)
#   lofar_add_executable(name)
#   lofar_add_library(name)
#   lofar_add_sbin_program(name)
#   lofar_add_test(name)
#   lofar_get_date(date)
#   lofar_get_hostname(name)
#
# Please refer to the module source for documentation of these macros.

#  $Id$
#
#  Copyright (C) 2008-2009
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

if(NOT DEFINED LOFAR_MACROS_INCLUDED)

  set(LOFAR_MACROS_INCLUDED TRUE)

  # --------------------------------------------------------------------------
  # force_option(option doc-string [value])
  #
  # The built-in CMake option() command will not let you modify an already set
  # option. With this macro you can forcibly set <option> to <value>.
  # --------------------------------------------------------------------------
  macro(force_option _option _doc)
    if(${ARGC} GREATER 3)
      message(SEND_ERROR 
        "force_option invoked with incorrect number of arguments")
    endif()
    string(TOUPPER "${ARGV2}" _value)
    # Test if value is really ON like option() does; 
    # ref. CMake sources cmOptionCommand.cxx and cmSystemTools.cxx
    if( _value STREQUAL "ON" OR
        _value STREQUAL "1" OR
        _value STREQUAL "YES" OR
        _value STREQUAL "TRUE" OR
        _value STREQUAL "Y" )
      set(${_option} ON CACHE BOOL "${_doc}" FORCE)
    else()
      set(${_option} OFF CACHE BOOL "${_doc}" FORCE)
    endif()
  endmacro(force_option)


  # --------------------------------------------------------------------------
  # join_arguments(var)
  #
  # Join the arguments in the (semi-colon separated) list VAR into one space
  # separated string. The string will be returned in the variable VAR.
  # This command is the opposite of the built-in command separate_arguments().
  # --------------------------------------------------------------------------
  macro(join_arguments var)
    set(_var)
    foreach(_v ${${var}})
      set(_var "${_var} ${_v}")
    endforeach(_v ${${var}})
    string(STRIP ${_var} _var)
    set(${var} ${_var})
  endmacro(join_arguments)


  # --------------------------------------------------------------------------
  # list_append_if(condition var value1..valuen)
  #
  # Apppend the values VALUE1 upto VALUEN to the list VAR if CONDITION is TRUE.
  # --------------------------------------------------------------------------
  macro(list_append_if _cond _list)
    if(${_cond})
      list(APPEND ${_list} ${ARGN})
    endif(${_cond})
  endmacro(list_append_if _cond _list)


  # --------------------------------------------------------------------------
  # lofar_add_bin_program(name)
  #
  # Add <name> to the list of programs that need to be compiled, linked and
  # installed into the <prefix>/bin directory.
  # --------------------------------------------------------------------------
  macro(lofar_add_bin_program _name)
    lofar_add_executable(${_name} ${ARGN})
    install(TARGETS ${_name} DESTINATION bin)
  endmacro(lofar_add_bin_program _name)


  # --------------------------------------------------------------------------
  # lofar_add_executable(name)
  #
  # Add an executable like add_executable() does.
  # Furthermore:
  # - Set the link dependencies of this executable on other LOFAR libraries
  #   using the information in ${PACKAGE_NAME}_LIBRARIES.
  # - Add a dependency of the current package on this executable.
  #
  # Note: since the libraries of the current package already have all their
  # link dependencies setup correctly (using lofar_add_library()), executables
  # only need to link to the libraries of the current package.
  # --------------------------------------------------------------------------
  macro(lofar_add_executable _name)
    add_executable(${_name} ${ARGN})
    get_property(_libs GLOBAL PROPERTY ${PACKAGE_NAME}_LIBRARIES)
    target_link_libraries(${_name} ${_libs})
    add_dependencies(${PACKAGE_NAME} ${_name})
  endmacro(lofar_add_executable _name)


  # --------------------------------------------------------------------------
  # lofar_add_library(name)
  #
  # Add a library like add_library() does. 
  # Furthermore:
  # - add the library to the list of libraries for the current package
  #   (global property ${PACKAGE_NAME}_LIBRARIES). 
  # - set the link dependencies of this library on other LOFAR libraries 
  #   using the information in ${PACKAGE_NAME}_DEPENDENCIES.
  # - mark the library for install into LOFAR_LIBDIR.
  # - add a dependency of the current package on the library.
  #
  # Note: link dependencies are determined by examining the link dependencies
  # of the libraries in the LOFAR packages that the current package depends
  # on. For this to work, each package must have have been defined using
  # lofar_package().
  # --------------------------------------------------------------------------
  macro(lofar_add_library _name)
    add_library(${_name} ${ARGN})
    if(NOT "${ARGN}" MATCHES "^MODULE")
      set_property(GLOBAL APPEND PROPERTY ${PACKAGE_NAME}_LIBRARIES ${_name})
    endif(NOT "${ARGN}" MATCHES "^MODULE")
    set(_link_libs)
    foreach(_dep ${${PACKAGE_NAME}_DEPENDENCIES})
      get_property(_dep_libs GLOBAL PROPERTY ${_dep}_LIBRARIES)
      list(APPEND _link_libs ${_dep_libs})
    endforeach(_dep ${${PACKAGE_NAME}_DEPENDENCIES})
    target_link_libraries(${_name} ${_link_libs} ${LOFAR_EXTRA_LIBRARIES})
#    set_target_properties(${_name} PROPERTIES 
#      VERSION ${${PACKAGE_NAME}_VERSION}
#      OUTPUT_NAME lofar_${_name})
    install(TARGETS ${_name} DESTINATION ${LOFAR_LIBDIR})
    add_dependencies(${PACKAGE_NAME} ${_name})
  endmacro(lofar_add_library _name)


  # --------------------------------------------------------------------------
  # lofar_add_sbin_program(name)
  #
  # Add <name> to the list of programs that need to be compiled, linked and
  # installed into the <prefix>/sbin directory.
  # --------------------------------------------------------------------------
  macro(lofar_add_sbin_program _name)
    lofar_add_executable(${_name} ${ARGN})
    install(TARGETS ${_name} DESTINATION sbin)
  endmacro(lofar_add_sbin_program _name)


  # --------------------------------------------------------------------------
  # lofar_add_test(name [source ...] [DEPENDS depend ...])
  #
  # Add a test like add_test() does.
  # Furthermore:
  # - If one or more sources are specfied, then instruct CMake how to compile
  #   and link the test program (this will implicitly create a target <name>);
  #   else create a custom target <name>.
  # - If one or more dependencies are specified, then add a dependency for
  #   <name> on each target <depend>.
  # - If there's a shell script <name>.sh, then add it to the list of tests;
  #   else just add the executable <name>.
  # - Adds a dependency for this test on the global target 'check', so that
  #   it will be compiled, linked and run when you do a 'make check'.
  # --------------------------------------------------------------------------
  macro(lofar_add_test _name)
    if(BUILD_TESTING)
      string(REGEX REPLACE ";?DEPENDS.*" "" _srcs "${ARGN}")
      string(REGEX MATCH "DEPENDS;.*" _deps "${ARGN}")
      string(REGEX REPLACE "^DEPENDS;" "" _deps "${_deps}")
      if(_srcs MATCHES ".+")
        lofar_add_executable(${_name} ${_srcs})
      else()
        add_custom_target(${_name})
      endif(_srcs MATCHES ".+")
      if(_deps MATCHES ".+")
        add_dependencies(${_name} ${_deps})
      endif(_deps MATCHES ".+")
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.sh)
        add_test(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.sh)
      else()
        add_test(${_name} ${_name})
      endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.sh)
      add_dependencies(check ${_name})
    endif(BUILD_TESTING)
  endmacro(lofar_add_test _name)


  # --------------------------------------------------------------------------
  # lofar_get_date(date)
  #
  # Return the current date and time in the variable DATE.
  # --------------------------------------------------------------------------
  macro(lofar_get_date _date)
    execute_process(COMMAND date
      OUTPUT_VARIABLE ${_date}
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endmacro(lofar_get_date _date)


  # --------------------------------------------------------------------------
  # lofar_get_hostname(name)
  #
  # Return the machine name (hostname) in the variable _hostname.
  # --------------------------------------------------------------------------
  macro(lofar_get_hostname _hostname)
    execute_process(COMMAND hostname -s
      OUTPUT_VARIABLE ${_hostname}
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endmacro(lofar_get_hostname _hostname)

endif(NOT DEFINED LOFAR_MACROS_INCLUDED)
