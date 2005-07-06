//#  WH_SBSplit.h: Splits the data according to subband and packs more times.
//#
//#  Copyright (C) 2002-2005
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

#ifndef TFLOPCORRELATOR_WH_SBSPLIT_H
#define TFLOPCORRELATOR_WH_SBSPLIT_H


#include <Common/KeyValueMap.h>
#include <tinyCEP/WorkHolder.h>


namespace LOFAR
{
  class WH_SBSplit: public WorkHolder
  {
  public:

    explicit WH_SBSplit(const string& name, const ACC::APS::ParameterSet pset);
    virtual ~WH_SBSplit();
    
    static WorkHolder* construct(const string& name, 
				 const ACC::APS::ParameterSet& pset);

    virtual WH_SBSplit* make(const string& name);

    virtual void process();

  private:
    /// forbid copy constructor
    WH_SBSplit (const WH_SBSplit&);
    /// forbid assignment
    WH_SBSplit& operator= (const WH_SBSplit&);

    ACC::APS::ParameterSet itsPS;

  };
} // namespace LOFAR

#endif
