//# DH_CoarseDelay.h: dataholder to hold the delay information to perform
//#            station synchronizaion       
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_COARSE_DELAY_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_COARSE_DELAY_H


#include <lofar_config.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{

class DH_CoarseDelay: public DataHolder
{
public:
  explicit DH_CoarseDelay (const string &name, int nrRSPs);

  DH_CoarseDelay(const DH_CoarseDelay &);

  virtual ~DH_CoarseDelay();

  DataHolder *clone() const;

  // Allocate the buffers.
  virtual void init();

  // accessor functions to the blob data
  const int getDelay(int index) const;
  void setDelay(int index, int value);
 
 private:
  /// Forbid assignment.
  DH_CoarseDelay &operator = (const DH_CoarseDelay &);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  int *itsDelayPtr;
  int itsNrRSPs;
};


}
#endif 
