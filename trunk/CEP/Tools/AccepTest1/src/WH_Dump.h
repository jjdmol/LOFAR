//# WH_Dump.h: 
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
