//#  AH_InputSection.h: one line description
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

#ifndef LOFAR_CS1_INPUTSECTION_AH_INPUTSECTION_H
#define LOFAR_CS1_INPUTSECTION_AH_INPUTSECTION_H

// \file
// one line description.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <CEPFrame/ApplicationHolder.h>
#include <CS1_Interface/Stub_BGL.h>
#include <CS1_Interface/Stub_Delay.h>
#include <CS1_Interface/CS1_Parset.h>

namespace LOFAR 
{
  namespace CS1 
  {

    // \addtogroup CS1_InputSection
    // @{

    // Description of class.
    // This is the ApplicationHolder for the input section of the CS1 application
    // Its main purposes are: 1) buffering the station data in an circular
    // buffer, and 2) transpose the data over a fast interconnect (using
    // MPI_Alltoallv, as opposed to CEPframe connections in earlier versions).

    class AH_InputSection: public ApplicationHolder
    {
    public:
      AH_InputSection();
      virtual ~AH_InputSection();
      virtual void define(const LOFAR::KeyValueMap&);
      virtual void undefine();
      virtual void run(int nsteps);

    private:
      // Copying is not allowed
      AH_InputSection (const AH_InputSection& that);
      AH_InputSection& operator= (const AH_InputSection& that);

      //# Datamembers
      CS1_Parset    *itsCS1PS;
      Stub_Delay     *itsDelayStub;
      Stub_BGL	     *itsOutputStub;

      std::vector<unsigned>	itsInputNodes, itsOutputNodes;
      std::vector<WorkHolder *> itsWHs;
    };

    // @}

  } // namespace CS1
} // namespace LOFAR

#endif
