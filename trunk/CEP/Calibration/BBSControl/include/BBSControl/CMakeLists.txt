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
  AddStep.h
  BlobStreamableConnection.h
  CalSession.h
  CalSessionTransactors.h
  Command.h
  CommandResult.h
  CommandVisitor.h
  CorrectStep.h
  Exceptions.h
  FinalizeCommand.h
  GlobalProcessControl.h
  GlobalSolveController.h
  InitializeCommand.h
  KernelProcessControl.h
  LocalSolveController.h
  Messages.h
  MessageHandlers.h
  MultiStep.h
  NextChunkCommand.h
  NoiseStep.h
  PredictStep.h
  RecoverCommand.h
  RefitStep.h
  ShiftStep.h
  SingleStep.h
  SolverProcessControl.h
  SolveStep.h
  SolveTask.h
  Step.h
  Strategy.h
  SubtractStep.h
  SynchronizeCommand.h
  Types.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PROJECT_NAME})

set(${PROJECT_NAME}_INCLUDE_PATH_SUFFIX ${PROJECT_NAME} 
  CACHE INTERNAL "Include path suffix for package \"${PROJECT_NAME}\"")
