//  DH_ExampleSim.h: Example DataHolder class
//
//  Copyright (C) 2000, 2001
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
//
//////////////////////////////////////////////////////////////////////

#ifndef EXAMPLESIM_DH_EXAMPLESIM_H
#define EXAMPLESIM_DH_EXAMPLESIM_H

#include <Common/lofar_complex.h>
#include <Transport/DataHolder.h>
#include <Transport/BaseSim.h>

/**
   This class is a DataHolder example class used in ExampleSim.
*/

class DH_ExampleSim: public LOFAR::DataHolder
{
public:
  
  explicit DH_ExampleSim (const string& name);
  DH_ExampleSim (const DH_ExampleSim&);
  virtual ~DH_ExampleSim();

  DataHolder* clone() const;

  void setCounter (int counter);
  int getCounter() const;

  int* getBuffer();
  const int* getBuffer() const;

  virtual void preprocess();

  virtual void postprocess();

private:
  /// Forbid assignment.
  DH_ExampleSim& operator= (const DH_ExampleSim&);

 // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*  itsCounter;
  int*  itsBuffer;
};


inline void DH_ExampleSim::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_ExampleSim::getCounter() const
  { return *itsCounter; }

inline int* DH_ExampleSim::getBuffer()
  { return itsBuffer; }

inline const int* DH_ExampleSim::getBuffer() const
  { return itsBuffer; }


#endif 
