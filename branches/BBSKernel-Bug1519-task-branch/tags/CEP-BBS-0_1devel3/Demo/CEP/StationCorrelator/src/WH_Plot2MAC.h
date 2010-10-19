//# WH_Plot2MAC.h: 
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef STATIONCORRELATOR_WH_PLOT2MAC_H
#define STATIONCORRELATOR_WH_PLOT2MAC_H

#include <sys/time.h>

#include <lofar_config.h>
#include <tinyCEP/WorkHolder.h>
#include <Blob/KeyValueMap.h>
#include <GCF/PALlight/CEPPropertySet.h>


using namespace std;
namespace LOFAR
{

class WH_Plot2MAC: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  explicit WH_Plot2MAC (const string& name,
			const KeyValueMap& kvm);

  virtual ~WH_Plot2MAC();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, 
				const KeyValueMap& kvm);

  /// Make a fresh copy of the WH object.
  virtual WH_Plot2MAC* make (const string& name);

  virtual void preprocess();
  /// Do a process step.
  virtual void process();
  virtual void postProcess();

private:
  /// Forbid copy constructor.
  WH_Plot2MAC (const WH_Plot2MAC&);

  /// Forbid assignment.
  WH_Plot2MAC& operator= (const WH_Plot2MAC&);
  
  int itsFBW; // frequency bandwidth of the DH_Beamlet 

  string itsOutputFileName;
  int itsOutputFile;
  
  int itsNelements;
  int itsNchannels;
  int itsNpolarisations;
    
  struct timeval itsLastTime;

  float itsBandwidth;    // stores 'measured' bandwidth in bytes/sec

  KeyValueMap itsKvm;
  GCF::CEPPMLlight::CEPPropertySet* itsPlotPS;
};

} // namespace LOFAR

#endif
