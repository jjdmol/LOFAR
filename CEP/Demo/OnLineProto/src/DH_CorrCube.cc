//  DH_CorrCube.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////


#include "DH_CorrCube.h"
#include <Common/KeyValueMap.h>

namespace LOFAR
{

DH_CorrCube::DH_CorrCube (const string& name, const ACC::ParameterSet& ps)
: DataHolder    (name, "DH_CorrCube"),
  itsBufferptr  (0),
  itsPS         (ps)
{
  itsBufSize = 
      itsPS.getInt("general.nstations")
    * itsPS.getInt("corr.fsize")
    * itsPS.getInt("corr.tsize") 
    * sizeof( complex<float> );
}

DH_CorrCube::DH_CorrCube(const DH_CorrCube& that)
  : DataHolder(that),
    itsBufferptr(0),
    itsPS(that.itsPS)
{
  itsBufSize = that.itsBufSize;
}

DH_CorrCube::~DH_CorrCube()
{
}

DataHolder* DH_CorrCube::clone() const
{
  return new DH_CorrCube(*this);
}

void DH_CorrCube::preprocess()
{
  // Add the fields to the data definition.
  addField("BeamID",BlobField< int >(1,1));
  addField("StartFrequency",BlobField< int >(1,1));
  addField("StartSeqNoptr",BlobField< int >(1,1));
  addField("FBW",BlobField< int >(1,1));
  addField("Buffer", 
	   BlobField<complex<float> >(1, //version 
				      itsBufSize)); //no_elements
  

  // Create the data blob (which calls fillPointers).
  createDataBlock();

}

void DH_CorrCube::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsBeamIDptr         = getData<int> ("BeamID");
  itsStartFrequencyptr = getData<int> ("StartFrequency");
  itsStartSeqNoptr     = getData<int> ("StartSeqNoptr");
  itsFBWptr            = getData<int> ("FBW");
  itsBufferptr         = getData<complex<float> > ("Buffer");
}

void DH_CorrCube::postprocess()
{
  itsBeamIDptr = 0;
  itsStartFrequencyptr = 0;
  itsStartSeqNoptr = 0;
  itsFBWptr = 0;
  itsBufferptr     = 0;
}

}
