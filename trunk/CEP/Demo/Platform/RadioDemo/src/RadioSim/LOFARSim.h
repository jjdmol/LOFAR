#ifndef _LOFARSIM_H__
#define _LOFARSIM_H__

#include "Simulator.h"

class LSFiller;


class LOFARSim: public Simulator
{
public:
  LOFARSim();

  void define(const ParamBlock&);
  void run(int nsteps = 10);
  void dump() const;
  void quit();


private:
  LSFiller* itsFiller;
};


#endif
