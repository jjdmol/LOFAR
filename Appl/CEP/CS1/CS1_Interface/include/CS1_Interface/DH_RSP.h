//# DH_RSP.h: DataHolder storing RSP raw ethernet frames for 
//#           StationCorrelator demo
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_CS1_INTERFACE_DH_RSP_H
#define LOFAR_CS1_INTERFACE_DH_RSP_H

#include <APS/ParameterSet.h>
#include <Transport/DataHolder.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_Interface/RectMatrix.h>
#include <CS1_Interface/SparseSet.h>

namespace LOFAR
{
  namespace CS1
  {

    class DH_RSP: public DataHolder
    {
    public:
      typedef INPUT_SAMPLE_TYPE BufferType;

      explicit DH_RSP (const string &name,
                       const ACC::APS::ParameterSet &pset);

      DH_RSP(const DH_RSP&);

      virtual ~DH_RSP();

      DataHolder* clone() const;

      /// Allocate the buffers.
      virtual void init();

      /// Accessor functions
      const int getStationID() const;
      void setStationID(int);
      const timestamp_t getTimeStamp() const;
      void setTimeStamp(timestamp_t);
      float getFineDelayAtBegin() const;
      void  setFineDelayAtBegin(float delay);
      float getFineDelayAfterEnd() const;
      void  setFineDelayAfterEnd(float delay);

      BufferType* getSubband(uint subbandNr);

      /// Reset the buffer
      void resetBuffer();
  
      RectMatrix<BufferType>& getDataMatrix() const;

      SparseSet &getFlags();
      const SparseSet &getFlags() const;

      /// (un)marshall flags into/from blob
      void getExtraData(), fillExtraData();

    private:
      /// Forbid assignment.
      DH_RSP& operator= (const DH_RSP&);

      // Fill the pointers (itsBuffer) to the data in the blob.
      virtual void fillDataPointers();

      /// pointers to data in the blob
      BufferType*  itsBuffer;
      SparseSet *itsFlags;
      int* itsStationID;
      float* itsDelays;
      timestamp_t* itsTimeStamp;

      int itsNTimes;
      int itsNoPolarisations;
      int itsNSubbands;
      unsigned int itsBufSize;

      const ACC::APS::ParameterSet &itsPSet;

      RectMatrix<BufferType> *itsMatrix;
    };

    inline DH_RSP::BufferType* DH_RSP::getSubband(uint subbandNr)
    {
      dimType sbDim = itsMatrix->getDim("Subbands");
      RectMatrix<DH_RSP::BufferType>::cursorType cursor = itsMatrix->getCursor(subbandNr * sbDim);
      return itsMatrix->getBlock(cursor, sbDim, 1, itsNTimes * itsNoPolarisations);
    }

    inline const int DH_RSP::getStationID() const
    { return *itsStationID; }

    inline void DH_RSP::setStationID(int id)
    { *itsStationID = id; }

    inline const timestamp_t DH_RSP::getTimeStamp() const
    { return *itsTimeStamp; }

    inline void DH_RSP::setTimeStamp(timestamp_t timestamp)
    { *itsTimeStamp = timestamp; }

    inline void DH_RSP::resetBuffer()
    { memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); }

    inline RectMatrix<DH_RSP::BufferType> &DH_RSP::getDataMatrix() const 
    { return *itsMatrix; }

    inline SparseSet &DH_RSP::getFlags()
    { return *itsFlags; }

    inline const SparseSet &DH_RSP::getFlags() const
    { return *itsFlags; }

    inline float DH_RSP::getFineDelayAtBegin() const
    { return itsDelays[0]; }

    inline void DH_RSP::setFineDelayAtBegin(float delay)
    { itsDelays[0] = delay; }

    inline float DH_RSP::getFineDelayAfterEnd() const
    { return itsDelays[1]; }
    
    inline void DH_RSP::setFineDelayAfterEnd(float delay)
    { itsDelays[1] = delay; }

  } // namespace CS1

} // namespace LOFAR

#endif 
