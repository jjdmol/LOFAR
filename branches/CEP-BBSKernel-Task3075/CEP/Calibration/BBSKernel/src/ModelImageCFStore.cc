//# ModelImageCFStore.cc: 
//#
//#
//# Copyright (C) 2012
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
//# $Id: ModelImageCFStore.cc 20029 2012-04-23 15:07:23Z duscha $

//#include <synthesis/MeasurementComponents/CFStore.h>
#include <BBSKernel/ModelImageCFStore.h>

namespace LOFAR{
namespace BBS{

CFStore& CFStore::operator=(const CFStore& other)
{
  if (&other != this)
  {
    data=other.data; 
    rdata=other.rdata; 
    coordSys=other.coordSys; 
    sampling.assign(other.sampling);
    xSupport.assign(other.xSupport);
    ySupport.assign(other.ySupport);
    pa=other.pa;
  }
  return *this;
};

void CFStore::show(const char *Mesg, ostream& os)
{
  if (!null())
    {
      if (Mesg != NULL)
        os << Mesg << endl;
      os << "Data Shape: " << data->shape() << endl;
      os << "Sampling: " << sampling << endl;
      os << "xSupport: " << xSupport << endl;
      os << "ySupport: " << ySupport << endl;
      os << "PA = " << pa.get("deg") << endl
        ;
    }
};

void CFStore::resize(Int nw,  Bool retainValues)
{
  xSupport.resize(nw,retainValues); ySupport.resize(nw,retainValues);
  sampling.resize(1);
}
void CFStore::resize(IPosition imShape, Bool retainValues)
{
  if (imShape.nelements() > NWPOS)
    resize(imShape(NWPOS), retainValues);
  if ((imShape.nelements() > 0) && (!data.null()))
    data->resize(imShape,retainValues);
}

} // end BBS namespace
} // end LOFAR namespace