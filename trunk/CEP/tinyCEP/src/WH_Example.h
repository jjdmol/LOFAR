//#  WH_Example.h: Example WorkHolder class for tinyCEPFrame test programs.
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

#ifndef TINYCEP_TEST_WH_EXAMPLE_H
#define TINYCEP_TEST_WH_EXAMPLE_H

#include <lofar_config.h>

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/DH_Example.h>


namespace LOFAR
{
  
  class WH_Example: public WorkHolder
  {

  public:
    explicit WH_Example (const string& name = "aWH_Example",
			 unsigned int nin=1,
			 unsigned int nout=1,
			 unsigned int nbuffer=10);
    
    virtual ~WH_Example();
    
    static WorkHolder* construct (const string& name, int ninput, int noutput,
				  const KeyValueMap&);

    virtual WH_Example* make(const string& name);
    
    virtual void preprocess();
    virtual void process();

  private:
    WH_Example (const WH_Example&);
    WH_Example& operator= (const WH_Example&);
    
    DH_Example itsProto;

    int itsBufLength;
  };

} // namespace LOFAR

#endif
