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


# ----------------------------------------------------------------------------
# Generic CMake macros for LOFAR
#
# Defines the following macros:
#   join_arguments(var)
#   list_append_if(condition var value1..valuen)
#   lofar_get_date(date)
#   lofar_get_hostname(name)
# ----------------------------------------------------------------------------

if(NOT DEFINED LOFAR_MACROS_INCLUDED)

  set(LOFAR_MACROS_INCLUDED TRUE)

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
  # Return the machine name (hostname) in the variable NAME.
  # --------------------------------------------------------------------------
  macro(lofar_get_hostname _hostname)
    execute_process(COMMAND hostname -s
      OUTPUT_VARIABLE ${_hostname}
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endmacro(lofar_get_hostname _hostname)

endif(NOT DEFINED LOFAR_MACROS_INCLUDED)
