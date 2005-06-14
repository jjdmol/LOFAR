//#  WH_Correlator.h: generic correlator class 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//#  $Id$

#ifndef BGL_WH_CORRELATE_H
#define BGL_WH_CORRELATE_H

#include <sys/time.h>  // for gettimeofday
#include <tinyCEP/WorkHolder.h>

#include <DH_Vis.h>

namespace LOFAR
{

class WH_Correlator: public WorkHolder
{
 public:

  explicit WH_Correlator (const string& name);

  virtual ~WH_Correlator();

  static WorkHolder* construct (const string& name);

  virtual WH_Correlator* make (const string& name);
  
  virtual void process();
  virtual void dump();

  double getBandwidth() ;
  double getAggBandwidth() ;
  double getCorrPerf();

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

  struct timeval t_start;
  struct timeval t_stop;

  double bandwidth;
  double agg_bandwidth;
  double corr_perf;

};

 inline double WH_Correlator::getBandwidth() { return bandwidth; } 
 inline double WH_Correlator::getAggBandwidth() { return agg_bandwidth; } 
 inline double WH_Correlator::getCorrPerf() { return corr_perf; } 

} // namespace LOFAR

double timer();
#endif
