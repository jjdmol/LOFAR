//# DH_Delay.h: dataholder to hold the delay information 
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

#ifndef LOFAR_CS1_INTERFACE_DH_DELAY_H
#define LOFAR_CS1_INTERFACE_DH_DELAY_H

#include <Transport/DataHolder.h>

namespace LOFAR
{
  namespace CS1
  {
    class DH_Delay: public DataHolder
    {
    public:

      struct DelayInfo
      {
	DelayInfo() :
	  coarseDelay(0), fineDelayAtBegin(0), fineDelayAfterEnd(0) {}
	int32 coarseDelay;
	float fineDelayAtBegin;
	float fineDelayAfterEnd;
      };
      
      explicit DH_Delay (const string &name, uint nrDelays);

      DH_Delay(const DH_Delay &);

      virtual ~DH_Delay();

      DH_Delay *clone() const;

      // Allocate the buffers.
      virtual void init();

      // @{
      // Accessor functions for the C-array of DelayInfo.
      DelayInfo& operator[](uint idx);
      const DelayInfo& operator[](uint idx) const;
      // @}

    private:
      /// Forbid assignment.
      DH_Delay &operator = (const DH_Delay &);

      // Fill the pointers (itsBuffer) to the data in the blob.
      virtual void fillDataPointers();

      /// Pointer to data in the blob
      DelayInfo *itsDelays;
      
      // Number of DelayInfo objects.
      uint itsNrDelays;
    };
    
  } // namespace CS1
  
  // This method is needed to put DH_Delay::DelayInfo into a BlobField.
  // \note It must be declared and defined in namespace %LOFAR, otherwise name
  // lookup will fail.
  void dataConvert(DataFormat fmt, CS1::DH_Delay::DelayInfo* buf, uint nrval);
  
} // namespace LOFAR

#endif 
