//# WH_SimStation.h: 
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

#ifndef ONLINEPROTO_WH_SIMSTATION_H
#define ONLINEPROTO_WH_SIMSTATION_H

#define NINPUT_BEAMLETS 128

#include <lofar_config.h>

#include "tinyCEP/WorkHolder.h"
#include "OnLineProto/DH_Beamlet.h"
#include "OnLineProto/DH_TimeDist.h"
#include <Common/Lorrays.h>
#include <Common/lofar_complex.h>
#include <ACC/ParameterSet.h>
#include <Common/lofar_fstream.h>
#include <OnLineProto/definitions.h>

namespace LOFAR
{
class WH_SimStation: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  explicit WH_SimStation (const string& name,
			  const int nout,
			  const string& fileName,
			  const ACC::ParameterSet& ps,
			  const int ID);

  virtual ~WH_SimStation();

  /// Static function to create an object.
    static WorkHolder* construct (const string& name, 
				  const int nout,
				  const string fileName,
				  const ACC::ParameterSet& ps,
				  const int ID);

  /// Make a fresh copy of the WH object.
  virtual WH_SimStation* make (const string& name);
      
  /// Do a process step.
  virtual void process();
      
  /// Show the work holder on stdout.
  virtual void dump();
	  
 private:
   /// Forbid copy constructor.
   WH_SimStation (const WH_SimStation&);
   
   /// Forbid assignment.
   WH_SimStation& operator= (const WH_SimStation&);

   void ReadData ();
     
   string itsFileName;
   ifstream itsInputFile;
   ACC::ParameterSet itsPS;
   int itsID;
   complex<float>* itsData;
   int itsCounter;
};

}

#endif
