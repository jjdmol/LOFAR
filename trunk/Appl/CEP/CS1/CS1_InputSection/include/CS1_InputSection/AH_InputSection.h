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
#include <CEPFrame/Step.h>
#include <CS1_InputSection/Connector.h>
#include <CS1_Interface/Stub_CoarseDelay.h>
#include <CS1_Interface/Stub_BGL_Subband.h>

namespace LOFAR 
{
  namespace CS1_InputSection 
  {

    // \addtogroup CS1_InputSection
    // @{

    // Description of class.
    // This is the ApplicationHolder for the input section of the CS1 application
    // This applicationholder uses the CEPFrame library and is supposed to
    // connect to the BGLProcessing application Holder (using only tinyCEP). 
    // The interface between these is defined in  the SB_Stub class.
    // The InputSection receives data from RSP boards using the WH_RSPInput class.
    // This class has an internal buffer and sends output to the WH_SBCollect class.

    class AH_InputSection: public ApplicationHolder
    {
    public:
      AH_InputSection();
      virtual ~AH_InputSection();
      virtual void undefine();
      virtual void define  (const LOFAR::KeyValueMap&);
      virtual void prerun  ();
      virtual void run     (int nsteps);
      virtual void dump    () const;
      virtual void quit    ();

    private:
      // Copying is not allowed
      AH_InputSection (const AH_InputSection& that);
      AH_InputSection& operator= (const AH_InputSection& that);

      //# Datamembers
      vector<Step*> itsSteps;
      Connector itsConnector;
      Stub_BGL_Subband *itsOutputStub;
      Stub_CoarseDelay *itsInputStub;
    };

    // @}

  } // namespace CS1_InputSection
} // namespace LOFAR

#endif
