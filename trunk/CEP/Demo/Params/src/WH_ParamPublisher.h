//#  WH_ParamPublisher.h: This workholder
//#
//#  Copyright (C) 2002-2003
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

#if !defined(WH_PARAMPUBLISHER_H)
#define WH_PARAMPUBLISHER_H

//# Includes
#include "CEPFrame/WorkHolder.h"

//# Forward Declarations


// Description of class.

class WH_ParamPublisher: public WorkHolder
{
 public:
  WH_ParamPublisher(const string& name);

  virtual ~WH_ParamPublisher();

  virtual WorkHolder* make(const string& name);

  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_ParamPublisher (const WH_ParamPublisher&);

  /// Forbid assignment.
  WH_ParamPublisher& operator= (const WH_ParamPublisher&);

  // Iteration number
  int itsIteration; 

};


#endif
