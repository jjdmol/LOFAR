//#  Correlator.h: Round robin correlator based on the premise that 
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

#ifndef CORRELATOR_CORRELATOR_H
#define CORRELATOR_CORRELATOR_H


#include <fstream>
#include <stdlib.h>
#include <string.h>

#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>

#include <Transport/TH_MPI.h>
//#include <Transport/TH_Socket.h>
#include "TH_Socket.h"    // local Socket Transport Holder. This is a workaround for the BGL socket bugs.

#include <tinyCEP/SimulatorParseClass.h>
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>

#include <WH_Correlator.h>
#include <WH_Random.h>
#include <WH_Dump.h>

#include <TestRange.h>
#include <Config.h>

using namespace std;

ifstream config_file;

char config_buffer[1024*10]; // 10 kb

int nchannels;
int nelements;
int nsamples;
int nruns;
int baseport;

int parse_config();

namespace LOFAR 
{
  class Correlator: public LOFAR::TinyApplicationHolder {

  public:
    Correlator(int elements, int samples, int channels, char* frontend_ip, int baseport, int targets);
    virtual ~Correlator();
    
    // overload methods form the ApplicationHolder base class
    virtual void define (const KeyValueMap& params = KeyValueMap());
    void undefine();
    virtual void init();
    virtual void run(int nsteps);
    virtual void dump();
    virtual void postrun();
    virtual void quit();
  private:
    WorkHolder* itsWH;
    
    int   itsNelements;
    int   itsNsamples;
    int   itsNchannels;
    char* itsIP;
    int   itsBaseport;
    int   itsNtargets;
  };

} // namespace LOFAR
#endif
