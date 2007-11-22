//#  DH_VisArray.h: Stores an array of visibility matrices
//#
//#  Copyright (C) 2002-2005
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

#ifndef TFLOPCORR_DH_VISARRAY_H
#define TFLOPCORR_DH_VISARRAY_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <TFC_Interface/DH_Vis.h>

namespace LOFAR
{

class DH_VisArray: public DataHolder {

public:
  typedef fcomplex BufferType;
  
  /// constructor
  explicit DH_VisArray(const string& name, 
		       const LOFAR::ACC::APS::ParameterSet pSet);

  DH_VisArray(const DH_VisArray&);
  virtual ~DH_VisArray();
  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Get write access to the buffer
  BufferType* getBuffer();
  BufferType* getBufferElement(short vis);

  /// Get read access to the buffer
  const BufferType* getBuffer() const;
  
  const unsigned int getBufSize();

  /// Get the number of visibilities in the array
  const unsigned int getNumVis();
  
  void setCenterFreq(double freq, int index);
  double getCenterFreq(int index);
  
private:
  /// Forbid assignment
  DH_VisArray& operator= (const DH_VisArray&);

  ACC::APS::ParameterSet itsPS;
  BufferType* itsBuffer;
  unsigned int itsBufSize;

  short itsNVis;       // #visibilities in the array
  short itsNStations;  // #stations in a visibility
  short itsNPols;      // #polarisations 

  double* itsCenterFreqs; // contains the center frequencies of the repective vis matrices

  void fillDataPointers();
};


inline DH_VisArray::BufferType* DH_VisArray::getBuffer()
  { return itsBuffer; }

inline DH_VisArray::BufferType* DH_VisArray::getBufferElement(short vis)
  {
    DBGASSERTSTR(vis < itsNVis, "DH_VisArray::getBufferElement: trying to get vis with index >= itsNVis");
    return (itsBuffer + vis * sizeof(DH_Vis::BufferType)/sizeof(fcomplex));
  }

inline const DH_VisArray::BufferType* DH_VisArray::getBuffer() const 
  { return itsBuffer; }

inline const unsigned int DH_VisArray::getBufSize()
  { return itsBufSize; }

inline const unsigned int DH_VisArray::getNumVis() 
  { return itsNVis; }

inline void DH_VisArray::setCenterFreq(double freq, int index)
  { 
    DBGASSERTSTR(index < itsNVis, "DH_VisArray::pushCenterFreq: Trying to push more freqs than there are Vis'");
    *(itsCenterFreqs+index) = freq;
  }
   
inline double DH_VisArray::getCenterFreq(int index)
  {
    DBGASSERTSTR(index < itsNVis, "DH_VisArray::getCenterFreq: trying to get center freq at too large an index ");
    return *(itsCenterFreqs+index);
  }

}
#endif
