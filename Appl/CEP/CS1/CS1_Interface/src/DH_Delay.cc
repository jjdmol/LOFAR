//#  DH_Delay.cc: dataholder to hold the delay information
//#
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

#include <lofar_config.h>

#include <CS1_Interface/DH_Delay.h>

namespace LOFAR
{
  namespace CS1
  {

    DH_Delay::DH_Delay(const string &name, uint nrRSPs)
      : DataHolder     (name, "DH_Delay"),
        itsCoarseDelays(0),
        itsFineDelaysAtBegin(0),
        itsFineDelaysAfterEnd(0),
        itsNrRSPs      (nrRSPs)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }
  
    DH_Delay::DH_Delay(const DH_Delay &that)
      : DataHolder (that),
        itsCoarseDelays(that.itsCoarseDelays),
        itsFineDelaysAtBegin(that.itsFineDelaysAtBegin),
        itsFineDelaysAfterEnd(that.itsFineDelaysAfterEnd),
        itsNrRSPs  (that.itsNrRSPs)
    {   
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }

    DH_Delay::~DH_Delay()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }

    DataHolder *DH_Delay::clone() const
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return new DH_Delay(*this);
    }

    void DH_Delay::init()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // add the fields to the data definition
      addField ("CoarseDelay", BlobField<int>(1, itsNrRSPs));
      addField ("FineDelayAtBegin", BlobField<float>(1, itsNrRSPs));
      addField ("FineDelayAfterEnd", BlobField<float>(1, itsNrRSPs));
  
      // create the data blob
      createDataBlock();

      for (uint i=0; i<itsNrRSPs; i++) {
        itsCoarseDelays[i] = 0;
        itsFineDelaysAtBegin[i] = 0;
        itsFineDelaysAfterEnd[i] = 0;
      }

    }

    void DH_Delay::fillDataPointers() 
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      itsCoarseDelays = getData<int> ("CoarseDelay");
      itsFineDelaysAtBegin = getData<float> ("FineDelayAtBegin");
      itsFineDelaysAfterEnd = getData<float> ("FineDelayAfterEnd");
    }

    int DH_Delay::getCoarseDelay(uint rspBoard) const
    { 
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      ASSERTSTR(rspBoard < itsNrRSPs, "index is not within range");
      return itsCoarseDelays[rspBoard]; 
    }

    void DH_Delay::setCoarseDelay(uint rspBoard, int delay)
    { 
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      ASSERTSTR(rspBoard < itsNrRSPs, "index is not within range");
      itsCoarseDelays[rspBoard] = delay;
    }

    float DH_Delay::getFineDelayAtBegin(uint rspBoard) const
    { 
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      ASSERTSTR(rspBoard < itsNrRSPs, "index is not within range");
      return itsFineDelaysAtBegin[rspBoard]; 
    }

    void DH_Delay::setFineDelayAtBegin(uint rspBoard, float delay)
    { 
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      ASSERTSTR(rspBoard < itsNrRSPs, "index is not within range");
      itsFineDelaysAtBegin[rspBoard] = delay;
    }

    float DH_Delay::getFineDelayAfterEnd(uint rspBoard) const
    { 
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      ASSERTSTR(rspBoard < itsNrRSPs, "index is not within range");
      return itsFineDelaysAfterEnd[rspBoard]; 
    }

    void DH_Delay::setFineDelayAfterEnd(uint rspBoard, float delay)
    { 
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      ASSERTSTR(rspBoard < itsNrRSPs, "index is not within range");
      itsFineDelaysAfterEnd[rspBoard] = delay;
    }

  } // namespace CS1

}  // namespace LOFAR
