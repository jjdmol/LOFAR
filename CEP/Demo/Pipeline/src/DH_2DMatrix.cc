//  DH_2DMatrix.cc:
//
//  Copyright (C) 2000, 2001
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
//  $Log$
//  Revision 1.3  2003/12/08 10:39:19  ellen
//  %[ER: 4]%
//  Merge of branch ER_4_development
//
//  Revision 1.2.2.1  2003/07/11 09:50:39  ellen
//  Changed Pipeline example in order to work with CEPFrame with DataManager functionality.
//
//  Revision 1.2  2002/11/20 11:05:42  schaaf
//
//  %[BugId: 117]%
//
//  working initial version for Scali
//
//  Revision 1.1.1.1  2002/11/13 15:58:06  schaaf
//  %[BugId: 117]%
//
//  Initial working version
//
//  Revision 1.5  2002/08/19 20:41:36  schaaf
//  %[BugId: 11]%
//  Memory allocation in preprocess method
//
//  Revision 1.4  2002/05/24 08:08:32  schaaf
//  %[BugId: 11]%
//  remove ^M characters
//
//  Revision 1.3  2002/05/16 15:14:02  schaaf
//  Removed explicit setting of itsBuffer
//
//  Revision 1.2  2002/05/14 11:39:41  gvd
//  Changed for new build environment
//
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//////////////////////////////////////////////////////////////////////


#include "Pipeline/DH_2DMatrix.h"

using namespace LOFAR;

DH_2DMatrix::DataPacket::DataPacket() {
}


DH_2DMatrix::DH_2DMatrix (const string& name,
			  int Xsize, const string& Xname,
			  int Ysize, const string& Yname,
			  const string& Zname,
			  int pols)
  
  : DataHolder (name, "DH_2DMatrix"),
    itsXSize(Xsize),
    itsYSize(Ysize),
    itsPols(pols)
{
  // fill in the names of the variables
  TRACER4("Set names");
  itsXName = std::string(Xname);
  itsYName = std::string(Yname);
  itsZName = std::string(Zname);
  
  TRACER4("End of DH_2DMatrix C'tor");
}

DH_2DMatrix::DH_2DMatrix(const DH_2DMatrix& that)
  : DataHolder(that),
    //    itsDataPacket(that.itsDataPacket),
    itsXSize(that.itsXSize),
    itsYSize(that.itsYSize),
    itsPols(that.itsPols),
    itsXName(that.itsXName),
    itsYName(that.itsYName),
    itsZName(that.itsZName)
{
}

DataHolder* DH_2DMatrix::clone() const
{
  return new DH_2DMatrix(*this);
}

void DH_2DMatrix::preprocess(){
  // Create the DataPacket AND its buffer in contiguous memory
  
  // Determine the number of bytes needed for DataPacket and buffer.
  // the size is that of the DataPacket object, plus the size of the Buffer
  unsigned int size = sizeof(DataPacket) + ((itsXSize*itsYSize*itsPols) * sizeof(DH_2DMatrix::DataType));
  // allocate the memmory
  void* ptr = allocate(size+4); // extra 4 bytes to avoid problems with word allignment
  
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();
  
  for (int x=0; x<itsXSize; x++) {
    for (int y=0; y<itsYSize; y++) {
      for (int p=0; p<itsPols; p++ ) {
      *getBuffer(x,y,p) = 0;
      }
    }
  }
  // Initialize base class.
  setDataPacket (itsDataPacket, size);
  TRACER2("Created 2D matrix : " << itsDataPacket << "   size="  << size);

}

DH_2DMatrix::~DH_2DMatrix()
{
}

const int DH_2DMatrix::getXSize(){
  return itsXSize; 
}

const int DH_2DMatrix::getYSize() {
  return itsYSize; 
}

void DH_2DMatrix::dump() const 
{
   for (int pol=0; pol<2; pol++) {
      cout << endl << "Polarisation: " << pol ;
      for (int x=0; 
           x < std::min(10, 1);
           x++) {
        cout << endl 
             << "xname"
             << x << "   ";
        for (int y=0; 
             y < std::min(10, 1);
             y++) {
          cout << *getBuffer(x,y,pol) << " ";
        }
      }
    }

}
