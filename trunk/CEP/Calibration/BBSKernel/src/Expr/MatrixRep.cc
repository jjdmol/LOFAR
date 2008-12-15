//# MatrixRep.cc: Temporary matrix for Mns
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

void MatrixRep::fillRowWithProducts(dcomplex, dcomplex, int)
{
  throw (AipsError ("MatrixRep::fillRowWithProducts not implemented"));
}

} // namespace BBS
} // namespace LOFAR
