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

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DELAY_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DELAY_H

#include <Transport/DataHolder.h>

namespace LOFAR
{

class DH_Delay: public DataHolder
{
public:
  explicit DH_Delay (const string &name, uint nrRSPs);

  DH_Delay(const DH_Delay &);

  virtual ~DH_Delay();

  DataHolder *clone() const;

  // Allocate the buffers.
  virtual void init();

  // accessor functions to the blob data
  int   getCoarseDelay(uint station) const;
  void  setCoarseDelay(uint station, int delay);
  float getFineDelayAtBegin(uint station) const;
  void  setFineDelayAtBegin(uint station, float delay);
  float getFineDelayAfterEnd(uint station) const;
  void  setFineDelayAfterEnd(uint station, float delay);
 
 private:
  /// Forbid assignment.
  DH_Delay &operator = (const DH_Delay &);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  int *itsCoarseDelays;
  float *itsFineDelaysAtBegin;
  float *itsFineDelaysAfterEnd;
  uint itsNrRSPs;
};


}
#endif 
