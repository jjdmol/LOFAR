//#  RTCorrelator.h: Round robin correlator based on the premise that 
//#  BlueGene is a hard real-time system.
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

#ifndef RTCORRELATOR_RTCORRELATOR_H
#define RTCORRELATOR_RTCORRELATOR_H


#include <fstream>
#include <stdlib.h>
#include <string.h>

#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <Common/Debug.h>
#include <Common/LofarLogger.h>

#include <Transport/TH_MPI.h>
#include <Transport/TH_Socket.h>

#include <tinyCEP/SimulatorParseClass.h>
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>

#include <WH_Correlator.h>

#include <Config.h>

using namespace std;

ifstream config_file;

char config_buffer[1024*10]; // 10 kb
char* frontend_ip;

int nchannels;
int nelements;
int nsamples;
int nruns;
int baseport;

int parse_config();

namespace LOFAR 
{
  class RTCorrelator: public LOFAR::TinyApplicationHolder {

  public:
    RTCorrelator();
    virtual ~RTCorrelator();
    
    // overload methods form the ApplicationHolder base class
    virtual void define (const KeyValueMap& params = KeyValueMap());
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
