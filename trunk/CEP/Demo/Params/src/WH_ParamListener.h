//#  WH_ParamListener.h: This workholder
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

#if !defined(WH_PARAMLISTENER_H)
#define WH_PARAMLISTENER_H

//# Includes
#include "CEPFrame/WorkHolder.h"

//# Forward Declarations


// Description of class.

class WH_ParamListener: public LOFAR::WorkHolder
{
 public:
  enum UpdateMode{Latest, New};

  WH_ParamListener(const string& name, UpdateMode uMode);

  virtual ~WH_ParamListener();

  virtual WorkHolder* make(const string& name);

  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_ParamListener (const WH_ParamListener&);

  /// Forbid assignment.
  WH_ParamListener& operator= (const WH_ParamListener&);
  
  int itsIteration;

  UpdateMode itsUMode;

};


#endif
