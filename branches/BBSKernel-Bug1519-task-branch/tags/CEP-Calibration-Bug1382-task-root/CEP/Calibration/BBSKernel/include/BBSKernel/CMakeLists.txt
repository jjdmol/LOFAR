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
  BlobIOExtensions.h
  Evaluator.h
  Equator.h
  Exceptions.h
  Instrument.h
  Measurement.h
  MeasurementAIPS.h
  MetaMeasurement.h
  Model.h
  ModelConfig.h
  ParmManager.h
  ParmProxy.h
  Solver.h
  SolverInterfaceTypes.h
  Types.h
  VisData.h
  VisDimensions.h
  VisSelection.h
  Expr/AzEl.h
  Expr/Diag.h
  Expr/DFTPS.h
  Expr/Expr.h
  Expr/ExprParm.h
  Expr/ExternalFunction.h
  Expr/GaussianCoherence.h
  Expr/GaussianSource.h
  Expr/HamakerDipole.h
  Expr/JonesCMul2.h
  Expr/JonesCMul3.h
  Expr/JonesExpr.h
  Expr/JonesInvert.h
  Expr/JonesMul.h
  Expr/JonesMul2.h
  Expr/JonesMul3.h
  Expr/JonesNode.h
  Expr/JonesResult.h
  Expr/JonesSum.h
  Expr/JonesVisData.h
  Expr/LMN.h
  Expr/Matrix.h
  Expr/MatrixComplexArr.h
  Expr/MatrixComplexSca.h
  Expr/MatrixRealArr.h
  Expr/MatrixRealSca.h
  Expr/MatrixRep.h
  Expr/MatrixTmp.h
  Expr/MIM.h
  Expr/PhaseRef.h
  Expr/PhaseShift.h
  Expr/PiercePoint.h
  Expr/PointCoherence.h
  Expr/PointSource.h
  Expr/PValueIterator.h
  Expr/Request.h
  Expr/Result.h
  Expr/ResultVec.h
  Expr/Pool.h
  Expr/Source.h
  Expr/StatUVW.h
  Expr/YatawattaDipole.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PROJECT_NAME})
