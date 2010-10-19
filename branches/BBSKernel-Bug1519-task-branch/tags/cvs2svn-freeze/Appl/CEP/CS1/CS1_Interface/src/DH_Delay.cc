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
#include <Common/LofarLogger.h>
#include <Blob/BlobField.tcc>        // for BlobField template instantiation

namespace LOFAR
{

  // Data conversion routine for class DelayInfo.
  void dataConvert(DataFormat fmt, CS1::DH_Delay::DelayInfo* buf, uint nrval)
  {
    for (uint i = 0; i < nrval; i++) {
      dataConvert32    (fmt, &(buf[i].coarseDelay));
      dataConvertFloat (fmt, &(buf[i].fineDelayAtBegin));
      dataConvertFloat (fmt, &(buf[i].fineDelayAfterEnd));
    }
  }
    
  // Instantiate the BlobField template.
  template class BlobField<CS1::DH_Delay::DelayInfo>;

    
  namespace CS1
  {

    DH_Delay::DH_Delay(const string &name, uint nrDelays)
      : DataHolder   (name, "DH_Delay"),
	itsDelays    (0),
        itsNrDelays(nrDelays)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }
  
    DH_Delay::DH_Delay(const DH_Delay &that)
      : DataHolder   (that),
	itsDelays    (that.itsDelays),
        itsNrDelays(that.itsNrDelays)
    {   
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }

    DH_Delay::~DH_Delay()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }

    DH_Delay *DH_Delay::clone() const
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return new DH_Delay(*this);
    }

    void DH_Delay::init()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Add the fields to the data definition
      addField ("Delays", BlobField<DelayInfo>(1, itsNrDelays));
      
      // create the data blob
      createDataBlock();

      for (uint i = 0; i < itsNrDelays; i++) {
	itsDelays[i] = DelayInfo();
      }

    }

    DH_Delay::DelayInfo& DH_Delay::operator[](uint idx)
    {
      ASSERTSTR(idx < itsNrDelays, "index out of range");
      return itsDelays[idx];
    }
    

    const DH_Delay::DelayInfo& DH_Delay::operator[](uint idx) const
    {
      return const_cast<DH_Delay&>(*this).operator[](idx);
    }
    

    void DH_Delay::fillDataPointers() 
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      itsDelays = getData<DelayInfo> ("Delays");
    }

  } // namespace CS1

}  // namespace LOFAR
