//#  WH_SubBand.h: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2004
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

#ifndef LOFAR_TFLOPCORRELATOR_WHSUBBAND_H
#define LOFAR_TFLOPCORRELATOR_WHSUBBAND_H

#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  class WH_SubBand: public WorkHolder {
    
  public:
    typedef u16complex FilterType;

    explicit WH_SubBand (const string& name);
    virtual ~WH_SubBand();

    static WorkHolder* construct (const string& name);
    virtual WH_SubBand* make (const string& name);

    virtual void process();
    virtual void dump();
    
  private:
    /// forbid copy constructor
    WH_SubBand(const WH_SubBand&);
    
    /// forbid assignment
    WH_SubBand& operator= (const WH_SubBand&);

    int itsNtaps;
    int itsNcoeff;
    
    FilterType* delayPtr;
    FilterType* delayLine;

    FilterType* coeffPtr;
    FilterType* inputPtr;

    FilterType acc;

    void adjustDelayPtr();
  };

} // namespace LOFAR

#endif
