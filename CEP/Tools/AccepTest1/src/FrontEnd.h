//#  FrontEnd.h:
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

#ifndef RT_FRONTEND_H
#define RT_FRONTEND_H


#include <lofar_config.h>

//# includes
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>

#include <WH_Random.h>
#include <WH_Correlator.h>
#include <WH_Dump.h>

#include <TestRange.h>

int   nchannels;
int   nelements;
int   nsamples;
int   nruns;
int   baseport;

namespace LOFAR
{

  class FrontEnd: public LOFAR::TinyApplicationHolder {

  public:
    FrontEnd(bool frontend, int port, int elements, int samples, 
	     int channels, int runs, int targets);
    virtual ~FrontEnd();

    // overload methods form the ApplicationHolder base class
    virtual void define(const KeyValueMap& params = KeyValueMap());
    virtual void undefine();
    virtual void init();
    virtual void run(int nsteps);
    virtual void dump() const;
    virtual void quit();

  private:

    vector<WorkHolder*> itsWHs;

    int         itsWHcount;
    bool        isFrontEnd;
    int         itsPort;
    int         itsNelements;
    int         itsNsamples;
    int         itsNchannels;
    int         itsNruns;
    int         itsNtargets;
  };

} // namespace


#endif
