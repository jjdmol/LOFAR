//#  WH_TTDelay.h: MPI_Alltoall transpose
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

#ifndef TFLOPCORRELATOR_WH_TTDELAY_H
#define TFLOPCORRELATOR_WH_TTDELAY_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file filename.h
// one line description.

//# Includes
#include <Common/KeyValueMap.h>
#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  class WH_TTDelay: public WorkHolder
  {
  public:

    explicit WH_TTDelay(const string& name, KeyValueMap kvm);
    virtual ~WH_TTDelay();
    static WorkHolder* construct(const string& name, KeyValueMap kvm);
    virtual WH_TTDelay* make(const string& name);

    virtual void process();
  private:
    /// forbid copy constructor
    WH_TTDelay (const WH_TTDelay&);
    /// forbid assignment
    WH_TTDelay& operator= (const WH_TTDelay&);
   
    KeyValueMap itsKVM; 

  };

  // @}
} // namespace LOFAR

#endif
