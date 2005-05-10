//#  WH_FilterInput.h: input workholder for filter tester
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

#ifndef TFLOP_CORRELATOR_FILTERINPUT_H
#define TFLOP_CORRELATOR_FILTERINPUT_H

#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  class WH_FilterInput: public WorkHolder {

  public:
    explicit WH_FilterInput (const string& name);
    virtual ~WH_FilterInput();

    static WorkHolder* construct (const string& name);
    virtual WH_FilterInput* make (const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void dump();

  private:
    WH_FilterInput (const WH_FilterInput&);
    WH_FilterInput& operator= (const WH_FilterInput&);

    int itsNtaps;
    int itsSBID;
    int itsNStations;
    int itsCpF;
  };
} // namespace LOFAR

#endif
