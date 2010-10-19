//#  Signals.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Generator/Signals.h>

namespace LOFAR {
  namespace Generator {

    void Sig_Zero::fillNext(Data* dataptr)
    {
      memset(&dataptr->getBeamlet(0, 0, 0), 
	     0, 
	     dataptr->getNTimes() * dataptr->getNPols() * dataptr->getNSubbands() * sizeof(RSPDataType));
    }

    void Sig_Random::fillNext(Data* dataptr)
    {
      RSPDataType* p = &dataptr->getBeamlet(0, 0, 0);      
      for (int i = 0; i < dataptr->getNTimes() * dataptr->getNSubbands() * dataptr->getNPols(); i ++) {
	p[i] = makei16complex(randint16(), randint16());
      }
    }

    void Sig_MultiChrome::fillNext(Data* dataptr)
    {
      for (int time = 0; time < dataptr->getNTimes(); time++) {
	double totSinus = 0;
	double totCosinus = 0;
	vector<SignalInfo>::iterator sig;
	for (sig = itsSignals.begin(); sig != itsSignals.end(); sig++) {
	  sig->phase += itsDeltaPhase * sig->frequency;
	  double s, c;
	  sincos(sig->phase, &s, &c);
	  totSinus += sig->amplitude * s;
	  totCosinus += sig->amplitude * c;
	}
	dataptr->getBeamlet(time, 0, 0) = makei16complex(static_cast<int>(totCosinus), static_cast<int>(totSinus));
      }
    }

  } // namespace Generator
} // namespace LOFAR
