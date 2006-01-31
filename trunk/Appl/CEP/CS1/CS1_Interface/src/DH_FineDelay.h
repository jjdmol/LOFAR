//# DH_FineDelay.h: FineDelay DataHolder
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_FINE_DELAY_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_FINE_DELAY_H

#include <CS1_Interface/CS1_Config.h>
#include <Transport/DataHolder.h>


namespace LOFAR
{

class DH_FineDelay: public DataHolder
{
  // Each second of sampled station data must be delayed (phase corrected) by
  // a value that varies from delayAtBegin at the begin of the second to
  // delayAfterEnd one sample after the end of the second.  The actual delay
  // of a particular sample is a linear interpolation of the delay at the begin
  // and after the end.  There are NR_STATIONS delay intervals.
  // Delays are specified in seconds.

public:
  typedef struct
  {
    float delayAtBegin, delayAfterEnd;
  } DelayIntervalType;

  typedef DelayIntervalType AllDelaysType[NR_STATIONS];

  explicit DH_FineDelay(const string& name);

  DH_FineDelay(const DH_FineDelay&);

  virtual ~DH_FineDelay();

  DataHolder *clone() const;

  virtual void init();

  AllDelaysType *getDelays()
  {
    return itsDelays;
  }

  const AllDelaysType *getDelays() const
  {
    return itsDelays;
  }

  const size_t nrDelays() const
  {
    return NR_STATIONS;
  }

  void setTestPattern();
  
private:
  /// Forbid assignment.
  DH_FineDelay &operator = (const DH_FineDelay&);

  void fillDataPointers();

  AllDelaysType *itsDelays;
};


}
#endif 
