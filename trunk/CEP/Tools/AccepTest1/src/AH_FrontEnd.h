//#  AH_FrontEnd.h:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#ifndef AH_FRONTEND_H
#define AH_FRONTEND_H


#include <lofar_config.h>

//# includes
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>

#include <WH_Random.h>
#include <WH_Correlator.h>
#include <WH_Dump.h>

namespace LOFAR
{

  class AH_FrontEnd: public LOFAR::TinyApplicationHolder {

  public:
    AH_FrontEnd(int port, int elements, int samples, 
	     int channels, int polarisations, int runs, int targets);
    virtual ~AH_FrontEnd();

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
    int         itsPort;
    int         itsNelements;
    int         itsNsamples;
    int         itsNchannels;
    int         itsNpolarisations;
    int         itsNruns;
    int         itsNtargets;
  };

} // namespace


#endif
