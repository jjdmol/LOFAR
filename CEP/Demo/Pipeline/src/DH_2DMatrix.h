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
//  Revision 1.7  2002/08/19 20:40:48  schaaf
//  %[BugId: 11]%
//  Use preprocess method
//
//  Revision 1.6  2002/05/24 08:41:23  schaaf
//  %[BugId: 11]%
//  removed ^M characters
//
//  Revision 1.5  2002/05/23 15:36:59  schaaf
//
//  %[BugId: 11]%
//  assert -> dbgassert
//
//  Revision 1.4  2002/05/16 15:12:00  schaaf
//  removed bug in declaration of itsBuffer
//
//  Revision 1.3  2002/05/14 11:39:41  gvd
//  Changed for new build environment
//
//  Revision 1.2  2002/05/07 11:16:20  schaaf
//  changed indexing mistake in getBuffer()
//
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_DH_2DMATRIX_H
#define BASESIM_DH_2DMATRIX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/DataHolder.h"
#include "Common/Debug.h"

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

  /// Allocate the buffers.
  virtual void preprocess();
  
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

    int itsXOffset;
    int itsYOffset;
    int itsZ;
    int itsSize;

    int itsBuffer[];
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
  DbgAssertStr(x >= 0 && x < itsXSize , "x not in range");
  DbgAssertStr(y >= 0 && y < itsYSize , "y not in range");
  return &(itsDataPacket->itsBuffer[x*itsYSize+y]); 
}

inline const int* DH_2DMatrix::getBuffer(int x, int y) const { 
  DbgAssertStr(x >= 0 && x < itsXSize , "x not in range");
  DbgAssertStr(y >= 0 && y < itsYSize , "y not in range");
  return &(itsDataPacket->itsBuffer[x*itsYSize+y]); 
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
