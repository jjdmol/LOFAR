# - Extract information from a subversion working copy
# The module defines the following variables:
#  Subversion_SVN_EXECUTABLE - path to svn command line client
#  Subversion_VERSION_SVN - version of svn command line client
#  Subversion_FOUND - true if the command line client was found
# If the command line client executable is found two macros are defined:
#  Subversion_WC_INFO(<dir> <var-prefix> [ERROR_QUIET])
#  Subversion_WC_LOG(<dir> <var-prefix>)
# Subversion_WC_INFO extracts information of a subversion working copy at
# a given location. ERROR_QUIET causes the macro to suppress errors.
# The macro defines the following variables:
#  <var-prefix>_WC_FOUND - if <dir> is a working copy
#  <var-prefix>_WC_URL - url of the repository (at <dir>)
#  <var-prefix>_WC_ROOT - root url of the repository
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_INFO - output of command `svn info <dir>'
# Subversion_WC_LOG retrieves the log message of the base revision of a
# subversion working copy at a given location. This macro defines the 
# variable:
#  <var-prefix>_LAST_CHANGED_LOG - last log of base revision
# Example usage:
#  FIND_PACKAGE(Subversion)
#  IF(Subversion_FOUND)
#    Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Current revision is ${Project_WC_REVISION}")
#    Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Last changed log is ${Project_LAST_CHANGED_LOG}")
#  ENDIF(Subversion_FOUND)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Tristan Carel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

SET(Subversion_FOUND FALSE)
SET(Subversion_SVN_FOUND FALSE)

FIND_PROGRAM(Subversion_SVN_EXECUTABLE svn
  DOC "subversion command line client")
MARK_AS_ADVANCED(Subversion_SVN_EXECUTABLE)

IF(Subversion_SVN_EXECUTABLE)
  SET(Subversion_SVN_FOUND TRUE)
  SET(Subversion_FOUND TRUE)

  MACRO(Subversion_WC_INFO dir prefix)
    # the subversion commands should be executed with the C locale, otherwise
    # the message (which are parsed) may be translated, Alex
    SET(_Subversion_SAVED_LC_ALL "$ENV{LC_ALL}")
    SET(ENV{LC_ALL} C)

    EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} --version
      WORKING_DIRECTORY ${dir}
      OUTPUT_VARIABLE Subversion_VERSION_SVN
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} info ${dir}
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      ERROR_VARIABLE Subversion_svn_info_error
      RESULT_VARIABLE Subversion_svn_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Subversion_svn_info_result} EQUAL 0)
      SET(${prefix}_WC_FOUND FALSE)
      IF(NOT "${ARGV2}" STREQUAL "ERROR_QUIET")
        MESSAGE(SEND_ERROR "Command \"${Subversion_SVN_EXECUTABLE} info ${dir}\" failed with output:\n${Subversion_svn_info_error}")
      ENDIF(NOT "${ARGV2}" STREQUAL "ERROR_QUIET")
    ELSE(NOT ${Subversion_svn_info_result} EQUAL 0)
      SET(${prefix}_WC_FOUND TRUE)

      STRING(REGEX REPLACE "^(.*\n)?svn, version ([.0-9]+).*"
        "\\2" Subversion_VERSION_SVN "${Subversion_VERSION_SVN}")
      STRING(REGEX REPLACE "^(.*\n)?URL: ([^\n]+).*"
        "\\2" ${prefix}_WC_URL "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Repository Root: ([^\n]+).*"
        "\\2" ${prefix}_WC_ROOT "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*"
        "\\2" ${prefix}_WC_REVISION "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Author: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_AUTHOR "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Rev: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_REV "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Date: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_DATE "${${prefix}_WC_INFO}")

    ENDIF(NOT ${Subversion_svn_info_result} EQUAL 0)

    # restore the previous LC_ALL
    SET(ENV{LC_ALL} ${_Subversion_SAVED_LC_ALL})

  ENDMACRO(Subversion_WC_INFO)

  MACRO(Subversion_WC_LOG dir prefix)
    # This macro can block if the certificate is not signed:
    # svn ask you to accept the certificate and wait for your answer
    # This macro requires a svn server network access (Internet most of the time)
    # and can also be slow since it access the svn server
    EXECUTE_PROCESS(COMMAND
      ${Subversion_SVN_EXECUTABLE} log -r BASE ${dir}
      OUTPUT_VARIABLE ${prefix}_LAST_CHANGED_LOG
      ERROR_VARIABLE Subversion_svn_log_error
      RESULT_VARIABLE Subversion_svn_log_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Subversion_svn_log_result} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${Subversion_SVN_EXECUTABLE} log -r BASE ${dir}\" failed with output:\n${Subversion_svn_log_error}")
    ENDIF(NOT ${Subversion_svn_log_result} EQUAL 0)
  ENDMACRO(Subversion_WC_LOG)

ENDIF(Subversion_SVN_EXECUTABLE)

IF(NOT Subversion_FOUND)
  IF(Subversion_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Subversion was not found.")
  ELSE(Subversion_FIND_REQUIRED)
    IF(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Subversion was not found.")
    ENDIF(NOT Subversion_FIND_QUIETLY)
  ENDIF(Subversion_FIND_REQUIRED)
ENDIF(NOT Subversion_FOUND)

# FindSubversion.cmake ends here.
