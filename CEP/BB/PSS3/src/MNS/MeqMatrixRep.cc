//# MeqMatrixRep.cc: Temporary matrix for Mns
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

#include <PSS3/MNS/MeqMatrixRep.h>
#include <casa/Exceptions/Error.h>


namespace LOFAR {

int MeqMatrixRep::nctor = 0;
int MeqMatrixRep::ndtor = 0;


MeqMatrixRep::~MeqMatrixRep()
{
  ndtor--;
}

void MeqMatrixRep::poolDelete()
{
  throw (AipsError ("MeqMatrixRep::poolDelete()"));
}

bool MeqMatrixRep::isDouble() const
{
  return false;
}

const double* MeqMatrixRep::doubleStorage() const
{
  throw (AipsError ("MeqMatrixRep::doubleStorage()"));
}

const complex<double>* MeqMatrixRep::dcomplexStorage() const
{
  throw (AipsError ("MeqMatrixRep::dcomplexStorage()"));
}

MeqMatrixRep* MeqMatrixRep::posdiff (MeqMatrixRep&)
{
  throw (AipsError ("MeqMatrixRep::posdiff requires real arguments"));
}
MeqMatrixRep* MeqMatrixRep::posdiffRep (MeqMatrixRealSca&)
{
  throw (AipsError ("MeqMatrixRep::posdiff requires real arguments"));
}
MeqMatrixRep* MeqMatrixRep::posdiffRep (MeqMatrixRealArr&)
{
  throw (AipsError ("MeqMatrixRep::posdiff requires real arguments"));
}
MeqMatrixRep* MeqMatrixRep::tocomplex (MeqMatrixRep&)
{
  throw (AipsError ("MeqMatrixRep::tocomplex requires real arguments"));
}
MeqMatrixRep* MeqMatrixRep::tocomplexRep (MeqMatrixRealSca&)
{
  throw (AipsError ("MeqMatrixRep::tocomplex requires real arguments"));
}
MeqMatrixRep* MeqMatrixRep::tocomplexRep (MeqMatrixRealArr&)
{
  throw (AipsError ("MeqMatrixRep::tocomplex requires real arguments"));
}

}
