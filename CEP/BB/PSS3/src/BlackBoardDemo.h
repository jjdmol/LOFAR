//  BlackBoardDemo.h: An empty simulator  
//
//  Copyright (C) 2000, 2002
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
//
//////////////////////////////////////////////////////////////////////////

#ifndef PSS3_BLACKBOARDDEMO_H
#define PSS3_BLACKBOARDDEMO_H


#include <lofar_config.h>

#include <CEPFrame/ApplicationHolder.h>
#include <Common/KeyValueMap.h>


/**
   This is a Simulator class which demonstrates the use of a blackboard control
   architecture in combination with PSS3 calibration software.
*/
/** 

 1. In BlackBoardDemo.cc change 'databaseName' to your own database name.

 2. Get the correct Measurement Sets.
*/
namespace LOFAR
{

class BlackBoardDemo: public LOFAR::ApplicationHolder
{
public:
  BlackBoardDemo();
  virtual ~BlackBoardDemo();

  // overloaded methods from the Simulator base class
  virtual void define(const LOFAR::KeyValueMap& params = LOFAR::KeyValueMap());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();

  void undefine();

private:
  LOFAR::Step** itsKSSteps;      // Pointer to array of Knowledge Source steps
  LOFAR::Step** itsKSInSteps;    // Pointer to array of input steps to Knowledge Sources
  LOFAR::Step** itsKSOutSteps;   // Pointer to array of output steps to Knowledge Sources

  int itsNumberKS;       // Total number of Knowledge Sources
};

} // end namespace LOFAR

#endif
