//  DH_CorrectionMatrix.cc:
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


#include "DH_CorrectionMatrix.h"
#include <Common/KeyValueMap.h>

namespace LOFAR
{

  DH_CorrectionMatrix::DH_CorrectionMatrix (const string& name, 
					    const int nx, // ny = ?
					    const int ny) // nx = ?
: DataHolder            (name, "DH_CorrectionMatrix"),
  itsBufferptr          (0),
  itsNx                 (nx),
  itsNy                 (ny)
{
}

DH_CorrectionMatrix::DH_CorrectionMatrix(const DH_CorrectionMatrix& that)
  : DataHolder     (that),
    itsBufferptr   (0)
{
  itsNx = that.itsNx;
  itsNy = that.itsNy;

}

DH_CorrectionMatrix::~DH_CorrectionMatrix()
{
}

DataHolder* DH_CorrectionMatrix::clone() const
{
  return new DH_CorrectionMatrix(*this);
}

void DH_CorrectionMatrix::preprocess()
{
  addField("ElapsedTime",BlobField< float >(1,1));
  addField("Buffer", 
	   BlobField<complex<float> >(1, //version 
				      itsNx*itsNy)); //no_elements    
  // Create the data blob (which calls fillPointers).
  createDataBlock();
  
  // initialise the blob field with values from C'tor
  
}

void DH_CorrectionMatrix::postprocess()
{
  itsBufferptr      = 0;
  itsElapsedTimeptr = 0;
}

}
