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

# List of header files that will be installed.
set(inst_HEADERS
  Package__Version.h
  Axis.h
  AxisMapping.h
  Box.h
  Grid.h
  ParmValue.h
  ParmSet.h
  ParmMap.h
  ParmCache.h
  ParmDBMeta.h
  ParmDB.h
  ParmDBCasa.h
  ParmDBLocker.h
  Parm.h
  ParmFacadeRep.h
  ParmFacadeLocal.h
  ParmFacadeDistr.h
  ParmFacade.h
  SourceInfo.h
  SourceDB.h
  SourceDBCasa.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PROJECT_NAME})
