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

include(LofarPackageVersion)

lofar_add_library(bbskernel
  Package__Version.cc
  Evaluator.cc
  Equator.cc
  MeasurementAIPS.cc
  Model.cc
  ModelConfig.cc
  ParmManager.cc
  ParmProxy.cc
  Solver.cc
  SolverInterfaceTypes.cc
  VisData.cc
  VisDimensions.cc
  VisSelection.cc
  Expr/AzEl.cc
  Expr/Diag.cc
  Expr/DFTPS.cc
  Expr/Expr.cc
  Expr/ExprParm.cc
  Expr/ExternalFunction.cc
  Expr/GaussianCoherence.cc
  Expr/GaussianSource.cc
  Expr/HamakerDipole.cc
  Expr/JonesCMul2.cc
  Expr/JonesCMul3.cc
  Expr/JonesExpr.cc
  Expr/JonesInvert.cc
  Expr/JonesMul.cc
  Expr/JonesMul2.cc
  Expr/JonesMul3.cc
  Expr/JonesNode.cc
  Expr/JonesSum.cc
  Expr/JonesVisData.cc
  Expr/LMN.cc
  Expr/Matrix.cc
  Expr/MatrixComplexArr.cc
  Expr/MatrixComplexSca.cc
  Expr/MatrixRealArr.cc
  Expr/MatrixRealSca.cc
  Expr/MatrixRep.cc
  Expr/MatrixTmp.cc
  Expr/MIM.cc
  Expr/PhaseRef.cc
  Expr/PhaseShift.cc
  Expr/PiercePoint.cc
  Expr/PointCoherence.cc
  Expr/PointSource.cc
  Expr/Request.cc
  Expr/Result.cc
  Expr/ResultVec.cc
  Expr/Source.cc
  Expr/SpectralIndex.cc
  Expr/StatUVW.cc
  Expr/YatawattaDipole.cc)

lofar_add_bin_program(versionbbskernel versionbbskernel.cc)
