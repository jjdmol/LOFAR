//  DH_2DMatrix.h: DataHolder containing DataPacket with station-frequency matrix 
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
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_DH_2DMATRIX_H
#define BASESIM_DH_2DMATRIX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "DataHolder.h"
#include "Debug.h"

/**
   This class is a DataHolder with a (time,frequency) matrix in the
   DataPacket buffer.
*/

class DH_2DMatrix: public DataHolder
{
public:

  explicit DH_2DMatrix (const string& name,
			int Xsize, const string& Xname,
			int Ysize, const string& Xname,
			const string& Zname);

  virtual ~DH_2DMatrix();

  int* getBuffer(int x, int y);
  const int* getBuffer(int x, int y) const;
  
  void setZ (int z);
  const int getZ() const;

  void setXOffset(int Xoffset);
  const int getXOffset() const;

  void setYOffset(int Yoffset);
  const int getYOffset() const;

  const int getXSize();
  const int getYSize();

  const string& getXName();
  const string& getYName();
  const string& getZName();

protected:
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket() ;
    int* itsBuffer;

    int itsXOffset;
    int itsYOffset;
    int itsZ;
  };

private:
  /// Forbid copy constructor.
  DH_2DMatrix (const DH_2DMatrix&);
  /// Forbid assignment.
  DH_2DMatrix& operator= (const DH_2DMatrix&);

  DataPacket* itsDataPacket;
  int itsXSize;
  int itsYSize;

  string itsXName;
  string itsYName;
  string itsZName;
};


inline int* DH_2DMatrix::getBuffer(int x, int y) { 
  AssertStr(x >= 0 && x < itsXSize , "x not in range");
  AssertStr(y >= 0 && y < itsYSize , "y not in range");
  return &(itsDataPacket->itsBuffer[x*itsXSize+y]); 
}

inline const int* DH_2DMatrix::getBuffer(int x, int y) const { 
  AssertStr(x >= 0 && x < itsXSize , "x not in range");
  AssertStr(y >= 0 && y < itsYSize , "y not in range");
  return &(itsDataPacket->itsBuffer[x*itsXSize+y]); 
}

inline void DH_2DMatrix::setZ (int z){ 
  itsDataPacket->itsZ = z; 
}

inline const int DH_2DMatrix::getZ() const { 
  return itsDataPacket->itsZ; 
}

inline void DH_2DMatrix::setXOffset (int Xoffset){ 
  itsDataPacket->itsXOffset = Xoffset; 
}

inline const int DH_2DMatrix::getXOffset() const { 
  return itsDataPacket->itsXOffset; 
}

inline void DH_2DMatrix::setYOffset (int Yoffset){ 
  itsDataPacket->itsYOffset = Yoffset; 
}

inline const int DH_2DMatrix::getYOffset() const { 
  return itsDataPacket->itsYOffset; 
}

inline const string& DH_2DMatrix::getXName() {
  return itsXName;
}

inline const string& DH_2DMatrix::getYName() {
  return itsYName;
}

inline const string& DH_2DMatrix::getZName() {
  return itsZName;
}
#endif 
