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
  BufferType* getBufferElement(short vis,
			       short station1, 
			       short station2,
			       short pol);

  int getBufferOffset(short vis, 
		      short station1,
		      short station2,  
		      short pol);

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

inline int DH_VisArray::getBufferOffset(short vis,
					short station1,
					short station2,
					short pol)
  // Addressing:
  //
  // The buffer basically concatenates the databuffers from serveral 
  // DH_Vis dataholders. So the basic addressing is similar to the 
  // DH_Vis addressing, with an added factor to account for the other 
  // buffers. Within a DH_Vis buffer the addressing is as below:
  // 
  // First determine the start position of the (stationA,stationB) data:
  // start at "upper left" corner with stationA=stationB=0 and
  // call this column 0, row 0. 
  // now address each row sequentially and
  // start with with column0 for the next stationA
  // Finally multiply by 4 to account for all polarisations
  //  (sA,sB) -> (sA*sA+sA)/2+sB
  //
  // This is the start address for the (stationA,stationB) data
  // add pol word to get to the requested polarisation.
  {
    DBGASSERTSTR(vis < itsNVis, "DH_VisArray::getBufferOffset: trying to access vis with higher index than available");
    DBGASSERTSTR(station1 <= station2,"DH_VisArray::getBufferOffset: only lower part of the correlation matrix is accessible");

    // DH_Vis::itsBufSize = itsNPols*itsNPols * itsNStations*(itsNStations+1)/2
    return vis*(itsNPols*itsNPols * itsNStations*(itsNStations+1)/2) +
      (2*(station1*station1+station1)+4*station2)+pol;
    
  }

inline DH_VisArray::BufferType* DH_VisArray::getBufferElement(short vis, 
								   short station1,
								   short station2,
								   short pol)
  {
    return &itsBuffer[getBufferOffset(vis, station1, station2, pol)];
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
