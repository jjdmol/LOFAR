//#  WH_SBCollect.h: Joins all data (stations, pols) for a subband
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_CS1_INPUTSECTION_WH_SBCOLLECT_H
#define LOFAR_CS1_INPUTSECTION_WH_SBCOLLECT_H

// \file
// Joins all data (stations, pols) for a subband

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <APS/ParameterSet.h>

namespace LOFAR 
{
  namespace CS1_InputSection 
  {

    // \addtogroup CS1_InputSection
    // @{

    //# Forward Declarations
    //class forward;


    // This class collect the data from different input nodes and joins all data of one subband for a certain amount of time
    class WH_SBCollect : public WorkHolder
    {
    public:
      explicit WH_SBCollect(const string& name, int sbID, 
			    const ACC::APS::ParameterSet pset,
			    const int noutputs);
      virtual ~WH_SBCollect();
    
      static WorkHolder* construct(const string& name, int sbID, 
				   const ACC::APS::ParameterSet pset,
				   const int noutputs);
      virtual WH_SBCollect* make(const string& name);
      
      virtual void process();
      virtual void postprocess();

    private:
      // Copying is not allowed
      WH_SBCollect (const WH_SBCollect& that);
      WH_SBCollect& operator= (const WH_SBCollect& that);

      //# Datamembers
      ACC::APS::ParameterSet itsPS;
      int itsSubBandID;
      int itsCore;
    };

    // @}

  } // namespace CS1_InputSection
} // namespace LOFAR

#endif
