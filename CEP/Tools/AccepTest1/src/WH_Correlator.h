//#  WH_Correlator.h:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#ifndef BGL_WH_CORRELATE_H
#define BGL_WH_CORRELATE_H

#include <sys/time.h>  // for gettimeofday

#include <lofar_config.h>
#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{

class WH_Correlator: public WorkHolder
{
 public:

  explicit WH_Correlator (const string& name,
			  const int    elements, 
			  const int    samples,
			  const int    channels,
			  const int    polarisations,
			  const int    targets);

  virtual ~WH_Correlator();

  static WorkHolder* construct (const string& name, 
				const int    elements, 
				const int    samples, 
				const int    channels,
				const int    polarisations,
				const int    targets);

  virtual WH_Correlator* make (const string& name);
  
  virtual void process();
  virtual void dump();

 private:
  /// forbid copy constructor
  WH_Correlator (const WH_Correlator&);
  /// forbid assignment
  WH_Correlator& operator= (const WH_Correlator&);

  int itsNelements;
  int itsNsamples;
  int itsNchannels;
  int itsNpolarisations;
  int itsNtargets;

  int itsRank;
};

} // namespace LOFAR

double timer();
#endif
