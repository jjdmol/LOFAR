#  LofarVariants.cmake: include global and host-specific variants files.
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
#  $Id$

if(NOT DEFINED LOFAR_VARIANTS_INCLUDED)
  set(LOFAR_VARIANTS_INCLUDED TRUE CACHE BOOL "LOFAR_VARIANTS_INCLUDED" FORCE)
  
  message(STATUS "**** ENTER: LofarVariants.cmake ****")

  include(CMakeSettings)

  ## --------------------------------------------------------------------------
  ## Include global variants file.
  ## --------------------------------------------------------------------------
  message(STATUS "Loading global variants file")
  include(${LOFAR_ROOT}/CMake/variants/variants)
  
  ## --------------------------------------------------------------------------
  ## Include machine specific variants file, if present
  ## --------------------------------------------------------------------------
  execute_process(COMMAND hostname -s
    OUTPUT_VARIABLE hostname
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(variants_file ${LOFAR_ROOT}/CMake/variants/variants.${hostname})
  
  if (EXISTS ${variants_file})
    message(STATUS "Loading host-specific variants file: ${variants_file}")
    include(${variants_file})
  endif (EXISTS ${variants_file})
  
  message(STATUS "**** LEAVE: LofarVariants.cmake ****")

endif(NOT DEFINED LOFAR_VARIANTS_INCLUDED)
