//#  AH_BackEnd.h:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#ifndef LOFAR_ACCEPTTEST1_AH_BACKEND_H
#define LOFAR_ACCEPTTEST1_AH_BACKEND_H

//# includes
#include <tinyCEP/TinyApplicationHolder.h>
#include <Transport/TransportHolder.h>

#include <WH_Random.h>
#include <WH_Correlator.h>
#include <WH_Dump.h>


namespace LOFAR
{

  class AH_BackEnd: public LOFAR::TinyApplicationHolder {

  public:
    AH_BackEnd(int port, int elements, int samples, 
	     int channels, int polarisations, int runs, 
	       int targets, int targetgroups, bool blocking);
    virtual ~AH_BackEnd();

    // overload methods form the ApplicationHolder base class
    virtual void define(const KeyValueMap& params = KeyValueMap());
    virtual void undefine();
    virtual void init();
    virtual void run(int nsteps);
    virtual void dump() const;
    virtual void quit();

  private:

    vector<WorkHolder*> itsWHs;
    vector<Connection*> itsInConns;
    vector<TransportHolder*> itsInTHs;

    int         itsWHcount;
    int         itsPort;
    int         itsNelements;
    int         itsNsamples;
    int         itsNchannels;
    int         itsNpolarisations;
    int         itsNruns;
    int         itsNtargets;
    int         itsNtgroups;
    bool        itsBlocking;

    struct timeval starttime;
    struct timeval stoptime;
    
    double bandwidth;
  };

} // namespace


#endif
