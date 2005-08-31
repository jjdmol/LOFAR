//#  WH_Distribute.h: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef TFLOPCORR_WH_DISTRIBUTE_H
#define TFLOPCORR_WH_DISTRIBUTE_H

#include <tinyCEP/WorkHolder.h>
#include <APS/ParameterSet.h>

#include <TFC_Interface/DH_FIR.h>

namespace LOFAR {


class WH_Distribute: public WorkHolder
{
 public:

  explicit WH_Distribute(const string& name, ACC::APS::ParameterSet itsPset, int inputs, int outputs);
  virtual ~WH_Distribute();

  static WorkHolder* construct(const string& name, const ACC::APS::ParameterSet itsPset, int inputs, int outputs);
  virtual WH_Distribute* make(const string& name);

  virtual void preprocess();
  virtual void process();
  virtual void dump();

 private:
  /// forbid copy constructor
  WH_Distribute (const WH_Distribute&);
  /// forbid assignment
  WH_Distribute& operator= (const WH_Distribute&);

  int itsNinputs;
  int itsNoutputs;

  int itsNsamples;        // samples in a timeslice
  int itsNinputElements;  // the number of stations from the input section
  int itsNoutputElements; // the number of stations to correlate
  int itsNpolarisations;  
  
  ACC::APS::ParameterSet itsPS;

  DH_FIR::BufferType* in_ptr;
  DH_FIR::BufferType* out_ptr;
};

}// namespace LOFAR

#endif
