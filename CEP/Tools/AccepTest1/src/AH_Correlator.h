//#  AH_Correlator.h: Round robin correlator based on the premise that 
//#  BlueGene is a hard real-time system.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
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

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

#include <Transport/TH_Socket.h>

#include <tinyCEP/SimulatorParseClass.h>
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>

#include <WH_Correlator.h>
#include <WH_Random.h>
#include <WH_Dump.h>

#include <DH_CorrCube.h>
#include <DH_Vis.h>

using namespace std;

namespace LOFAR 
{
  class AH_Correlator: public LOFAR::TinyApplicationHolder {

  public:
    AH_Correlator(int elements, int samples, int channels, int polarisations, char* frontend_ip, int baseport, int targets);
    virtual ~AH_Correlator();
    
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
    int   itsNpolarisations;
    char* itsIP;
    int   itsBaseport;
    int   itsNtargets;
  };

} // namespace LOFAR
#endif
