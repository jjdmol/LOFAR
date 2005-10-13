//#  DH_VisArray: stores an array of correlation matrices
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <APS/ParameterSet.h>
#include <DH_VisArray.h>

namespace LOFAR
{

DH_VisArray::DH_VisArray(const string& name, 
			 const ACC::APS::ParameterSet pSet)
  : DataHolder      (name, "DH_VisArray"),
    itsPS           (pSet),
    itsBuffer       (0),
    itsBufSize      (0),
    itsCenterFreqs  (0)
{
  itsNVis = itsPS.getInt32("Storage.NVisPerInput");
  itsNStations = itsPS.getInt32("PPF.NrStations");
  itsNPols = itsPS.getInt32("PPF.NPolarizations");
}

DH_VisArray::DH_VisArray(const DH_VisArray& that)
  : DataHolder   (that),
    itsPS        (that.itsPS),
    itsBuffer    (that.itsBuffer),
    itsNVis      (that.itsNVis),
    itsNStations (that.itsNStations),
    itsNPols     (that.itsNPols)
{}
 
DH_VisArray::~DH_VisArray()
{}   
  
DataHolder* DH_VisArray::clone() const
{
  return new DH_VisArray(*this);
}

void DH_VisArray::init()
{
  // determine the size of the buffer
  // buffersize is itsNVis * DH_Vis::itsBufSize
  itsBufSize = itsNVis * sizeof(DH_Vis::BufferType) / sizeof(fcomplex);
  addField("Buffer", BlobField<BufferType>(1, itsBufSize));
  addField("CenterFreqs", BlobField<double>(1, itsNVis));
  createDataBlock();

  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType));
}

void DH_VisArray::fillDataPointers()
{
  itsBuffer = getData<BufferType> ("Buffer");
  itsCenterFreqs = getData<double> ("CenterFreqs");
}

} // namespace LOFAR
