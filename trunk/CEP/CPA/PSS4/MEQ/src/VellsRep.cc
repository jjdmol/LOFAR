//# VellsRep.cc: Temporary vells for Mns
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

#include <MEQ/VellsRep.h>
#include <aips/Exceptions/Error.h>

namespace MEQ {

int VellsRep::nctor = 0;
int VellsRep::ndtor = 0;


VellsRep::~VellsRep()
{
  ndtor--;
}

bool VellsRep::isReal() const
{
  return false;
}

double* VellsRep::realStorage()
{
  throw (AipsError ("MEQ::VellsRep::realStorage()"));
}

complex<double>* VellsRep::complexStorage()
{
  throw (AipsError ("MEQ::VellsRep::complexStorage()"));
}

VellsRep* VellsRep::posdiff (VellsRep&)
{
  throw (AipsError ("MEQ::VellsRep::posdiff requires real arguments"));
}
VellsRep* VellsRep::posdiffRep (VellsRealSca&)
{
  throw (AipsError ("MEQ::VellsRep::posdiff requires real arguments"));
}
VellsRep* VellsRep::posdiffRep (VellsRealArr&)
{
  throw (AipsError ("MEQ::VellsRep::posdiff requires real arguments"));
}
VellsRep* VellsRep::tocomplex (VellsRep&)
{
  throw (AipsError ("MEQ::VellsRep::tocomplex requires real arguments"));
}
VellsRep* VellsRep::tocomplexRep (VellsRealSca&)
{
  throw (AipsError ("MEQ::VellsRep::tocomplex requires real arguments"));
}
VellsRep* VellsRep::tocomplexRep (VellsRealArr&)
{
  throw (AipsError ("MEQ::VellsRep::tocomplex requires real arguments"));
}

} // namespace MEQ
