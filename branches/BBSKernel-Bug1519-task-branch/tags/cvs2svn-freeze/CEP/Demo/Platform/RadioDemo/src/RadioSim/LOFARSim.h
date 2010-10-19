#ifndef _LOFARSIM_H__
#define _LOFARSIM_H__

#include "Simulator.h"
#include "general.h"

class LSFiller;


class LOFARSim: public Simulator
{
public:
  LOFARSim();

  void define(const ParamBlock&);
  void run(int nsteps = 10);
  void dump() const;
  void quit();

  Step  *antenna[STATIONS][ELEMENTS];
  Step  *correlator[BEAMS*FCORR];

private:
  LSFiller* itsFiller;
  int itsRunInApp[10];
  int itsSkipApp1;
  int itsSkipApp2;
};


#endif
