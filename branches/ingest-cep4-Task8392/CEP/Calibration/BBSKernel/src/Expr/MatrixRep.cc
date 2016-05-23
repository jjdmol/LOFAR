//# MatrixRep.cc: Temporary matrix for Mns
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/Expr/MatrixRep.h>
#include <casa/Exceptions/Error.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{


MatrixRep::~MatrixRep()
{
}

const double* MatrixRep::doubleStorage() const
{
  throw (AipsError ("MatrixRep::doubleStorage()"));
}

void MatrixRep::dcomplexStorage(const double *&, const double *&) const
{
  throw (AipsError ("MatrixRep::dcomplexStorage()"));
}

MatrixRep* MatrixRep::posdiff (MatrixRep&)
{
  throw (AipsError ("MatrixRep::posdiff requires real arguments"));
}
MatrixRep* MatrixRep::posdiffRep (MatrixRealSca&)
{
  throw (AipsError ("MatrixRep::posdiff requires real arguments"));
}
MatrixRep* MatrixRep::posdiffRep (MatrixRealArr&)
{
  throw (AipsError ("MatrixRep::posdiff requires real arguments"));
}

MatrixRep* MatrixRep::tocomplex (MatrixRep&)
{
  throw (AipsError ("MatrixRep::tocomplex requires real arguments"));
}
MatrixRep* MatrixRep::tocomplexRep (MatrixRealSca&)
{
  throw (AipsError ("MatrixRep::tocomplex requires real arguments"));
}
MatrixRep* MatrixRep::tocomplexRep (MatrixRealArr&)
{
  throw (AipsError ("MatrixRep::tocomplex requires real arguments"));
}

MatrixRep* MatrixRep::min (MatrixRep&)
{
  throw (AipsError ("MatrixRep::min requires a real argument"));
}
MatrixRep* MatrixRep::minRep (MatrixRealSca&)
{
  throw (AipsError ("MatrixRep::min requires a real argument"));
}
MatrixRep* MatrixRep::minRep (MatrixRealArr&)
{
  throw (AipsError ("MatrixRep::min requires a real argument"));
}

MatrixRep* MatrixRep::max (MatrixRep&)
{
  throw (AipsError ("MatrixRep::max requires a real argument"));
}
MatrixRep* MatrixRep::maxRep (MatrixRealSca&)
{
  throw (AipsError ("MatrixRep::max requires a real argument"));
}
MatrixRep* MatrixRep::maxRep (MatrixRealArr&)
{
  throw (AipsError ("MatrixRep::max requires a real argument"));
}

MatrixRep* MatrixRep::atan2 (MatrixRep&)
{
  throw (AipsError ("MatrixRep::atan2 requires a real argument"));
}
MatrixRep* MatrixRep::atan2Rep (MatrixRealSca&)
{
  throw (AipsError ("MatrixRep::atan2 requires a real argument"));
}
MatrixRep* MatrixRep::atan2Rep (MatrixRealArr&)
{
  throw (AipsError ("MatrixRep::atan2 requires a real argument"));
}

MatrixRep* MatrixRep::asin()
{
  throw (AipsError ("MatrixRep::asin requires a real argument"));
}

MatrixRep* MatrixRep::acos()
{
  throw (AipsError ("MatrixRep::acos requires a real argument"));
}

void MatrixRep::fillRowWithProducts(dcomplex, dcomplex, int)
{
  throw (AipsError ("MatrixRep::fillRowWithProducts not implemented"));
}

} // namespace BBS
} // namespace LOFAR
