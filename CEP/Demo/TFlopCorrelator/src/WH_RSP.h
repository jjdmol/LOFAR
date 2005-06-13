//#  WH_RSP.h: 
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

#ifndef TFLOPCORRELATOR_WH_RSP_H
#define TFLOPCORRELATOR_WH_RSP_H

#include <ACC/ParameterSet.h>
#include <tinyCEP/WorkHolder.h>


namespace LOFAR
{
  class WH_RSP: public WorkHolder
  {
  public:

    explicit WH_RSP(const string& name, 
                    const ACC::ParameterSet pset);
    virtual ~WH_RSP();
    
    static WorkHolder* construct(const string& name, 
                                 const ACC::ParameterSet pset);
    virtual WH_RSP* make(const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void postprocess();

  private:
    /// forbid copy constructor
    WH_RSP (const WH_RSP&);
    /// forbid assignment
    WH_RSP& operator= (const WH_RSP&);


    ACC::ParameterSet itsPSet;
  };

} // namespace LOFAR

#endif
