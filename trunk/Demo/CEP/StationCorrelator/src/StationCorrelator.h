//#  StationCorrelator.h: 
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

#ifndef STATIONCORRELATOR_H
#define STATIONCORRELATOR_H

#include <CEPFrame/ApplicationHolder.h>

namespace LOFAR {
// Description of class.
class StationCorrelator: public LOFAR::ApplicationHolder
{
 public:
  StationCorrelator(KeyValueMap kvm);
  virtual ~StationCorrelator();
  virtual void undefine();
  virtual void define  (const LOFAR::KeyValueMap&);
  virtual void init    ();
  virtual void run     (int nsteps);
  virtual void dump    () const;
  virtual void quit    ();
 private:
  vector<WorkHolder*> itsWHs;

  unsigned int itsNrsp;
  unsigned int itsNcorrelator;
  unsigned int itsNdump;

  KeyValueMap itsKVM;
};
}
#endif
