//  WH_ReadSignals.h:
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//

#ifndef DATAGEN_WH_READSIGNAL_H
#define DATAGEN_WH_READSIGNAL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <DataGen/DH_SampleR.h>


class WH_ReadSignal : public WorkHolder 
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_ReadSignal (const string & name, 
				 unsigned int nout,
				 const string & signalFileName);

   virtual ~ WH_ReadSignal ();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, 
								int ninput,
								int noutput, 
								const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_ReadSignal* make (const string & name) const;

  /// Do a process step.
  virtual void process ();

  /// Show the work holder on stdout.
  virtual void dump () const;

  /// There are no input DataHolders.
  virtual DataHolder* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_SampleR* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
   WH_ReadSignal (const WH_ReadSignal&);

  /// Forbid assignment.
   WH_ReadSignal& operator = (const WH_ReadSignal&);

  /// Pointer to the array of output DataHolders.
  DH_SampleR** itsOutHolders;
  string       itsFileName;
  ifstream     itsFile;
};


#endif
