//#  AH_TestFilter.h: Application holder for FIR filter tester
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

#ifndef TFLOP_CORRELATOR_AH_TESTFILTER_H
#define TFLOP_CORRELATOR_AH_TESTFILTER_H

// global includes
#include <tinyCEP/TinyApplicationHolder.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <tinyCEP/WorkHolder.h>


// includes subject to test (WorkHolders to test etc.)
#include "../src/DH_SubBand.h"
#include "../src/WH_SubBand.h"
#include "../src/DH_CorrCube.h"

// local includes (test WorkHolders etc.)
#include <WH_FilterInput.h>
#include <WH_FilterOutput.h>

namespace LOFAR
{
  class AH_TestFilter: public LOFAR::TinyApplicationHolder {

  public:
    AH_TestFilter();
    virtual ~AH_TestFilter();

    // overload methods from the TinyApplicationHolder base class
    virtual void define (const KeyValueMap& params = KeyValueMap());
    void undefine();
    virtual void init();
    virtual void run(int nsteps);
    virtual void dump();
    virtual void postrun();
    virtual void quit();

  private:
    WorkHolder* itsWH;
  };
} // namespace LOFAR

#endif
