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
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//////////////////////////////////////////////////////////////////////


#include "Transpose/DH_2DMatrix.h"

DH_2DMatrix::DataPacket::DataPacket() {
}

DH_2DMatrix::DH_2DMatrix (const string& name,
			  int Xsize, const string& Xname,
			  int Ysize, const string& Yname,
			  const string& Zname)
  : DataHolder (name, "DH_2DMatrix"),
    itsXSize(Xsize),
    itsYSize(Ysize)
{
  // Create the DataPacket AND its buffer in contiguous memory
  
  // Determine the number of bytes needed for DataPacket and buffer.
  // the size is that of the DataPacket object, plus the size of the Buffer
  unsigned int size = sizeof(DataPacket) + ((Xsize*Ysize) * sizeof(int));
  // allocate the memmory
  char* ptr = new char[size];
  
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();

  // Fill in the buffer pointer and initialize the buffer.
  // the buffer starts after the DataPacket object
  itsDataPacket->itsBuffer = (int*)(ptr + sizeof(DataPacket)); 

  for (int x=0; x<Xsize; x++) {
    for (int y=0; y<Ysize; y++) {
      *getBuffer(x,y) = 0;
    }
  }
  
  // Initialize base class.
  setDataPacket (itsDataPacket, size);

  // fill in the names of the variables
  TRACER4("Set names");
  itsXName = std::string(Xname);
  itsYName = std::string(Yname);
  itsZName = std::string(Zname);

  TRACER4("End of C'tor");
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


