//# WH_Dump.h: 
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef BG_CORRELATOR_WH_DUMP_H
#define BG_CORRELATOR_WH_DUMP_H

#include <lofar_config.h>
#include <tinyCEP/WorkHolder.h>

using namespace std;
namespace LOFAR
{

class WH_Dump: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  explicit WH_Dump (const string& name,
		    const int    elements, 
		    const int    channels);

  virtual ~WH_Dump();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, 
				const int    elements,
				const int    channels);

  /// Make a fresh copy of the WH object.
  virtual WH_Dump* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_Dump (const WH_Dump&);

  /// Forbid assignment.
  WH_Dump& operator= (const WH_Dump&);
  
  int itsFBW; // frequency bandwidth of the DH_Beamlet 

  //  ofstream itsOutputFile;
  int itsIndex;
  int itsCounter;
  
  int itsNelements;
  int itsNchannels;
    
};

}

#endif
