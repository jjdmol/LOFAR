//#  WH_Random.h: a random generator for BG/L correlator
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

#ifndef BG_CORRELATOR_WH_RANDOM_H
#define BG_CORRELATOR_WH_RANDOM_H

#include <lofar_config.h>
#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  
  class WH_Random: public WorkHolder
  {
  public:
    explicit WH_Random (const string& name,
			unsigned int nin,
			unsigned int nout,
			const int FBW);

    virtual ~WH_Random();

    static WorkHolder* construct (const string& name,
				  unsigned int nin,
				  unsigned int nout,
				  const int FBW);
    
    virtual WH_Random* make (const string& name);
    
    //    virtual void preprocess();
    virtual void process();
    virtual void dump();
   
  private:
    WH_Random (const WH_Random&);
    WH_Random& operator= (const WH_Random&);

    int itsFBW;
    int itsIntegrationTime;
    float index;
  };

} // namespace LOFAR

#endif
