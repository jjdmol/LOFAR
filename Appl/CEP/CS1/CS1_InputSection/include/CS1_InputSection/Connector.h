//#  Connector.h: one line description
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

#ifndef LOFAR_CS1_INPUTSECTION_CONNECTOR_H
#define LOFAR_CS1_INPUTSECTION_CONNECTOR_H

// \file
// one line description.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Transport/TransportHolder.h>
#include <APS/ParameterSet.h>
#include <CEPFrame/Step.h>

namespace LOFAR 
{
  namespace CS1_InputSection 
  {

    // \addtogroup CS1_InputSection
    // @{

    //# Forward Declarations
    //class forward;


    // Description of class.
    class Connector
    {
    public:
      Connector();
      ~Connector();

      static TransportHolder* readTH(const ACC::APS::ParameterSet& ps, const string& key, const bool isReceiver = true);
      void connectSteps(Step* src, int srcDH, Step* dst, int dstDH);

    private:
      // Copying is not allowed
      Connector (const Connector& that);
      Connector& operator= (const Connector& that);

      //# Datamembers

    };

    // @}

  } // namespace CS1_InputSection
} // namespace LOFAR

#endif
