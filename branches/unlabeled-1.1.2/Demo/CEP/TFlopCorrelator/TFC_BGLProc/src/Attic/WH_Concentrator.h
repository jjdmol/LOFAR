//#  WH_Concentrator.h: receive data and concentrate it into one DH
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

#ifndef TFLOPCORR_WH_CONCENTRATOR_H
#define TFLOPCORR_WH_CONCENTRATOR_H

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <APS/ParameterSet.h>

#include <TFC_Interface/DH_Vis.h>
#include <TFC_Interface/DH_VisArray.h>

namespace LOFAR
{

class WH_Concentrator: public WorkHolder
{
 public:
  explicit WH_Concentrator (const string& name, ACC::APS::ParameterSet itsPS, int inputs);
  virtual ~WH_Concentrator();

  static WorkHolder* construct(const string& name, const ACC::APS::ParameterSet pset, int inputs);
  virtual WH_Concentrator* make(const string& name);

  virtual void preprocess();
  virtual void process();
  virtual void dump();

 private:
  /// forbid copy constructor 
  WH_Concentrator (const WH_Concentrator&);
  /// forbid assignment
  WH_Concentrator& operator= (const WH_Concentrator&);

  ACC::APS::ParameterSet itsPS;

  short itsNVis;
  short itsNStations;
  short itsNPols;
};


} // namespace LOFAR

#endif
