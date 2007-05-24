//#  AH_DelayCompensation.h: Application holder for the CS1 delay compensation
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

#ifndef LOFAR_CS1_DELAYCOMPENSATION_AH_DELAYCOMPENSATION_H
#define LOFAR_CS1_DELAYCOMPENSATION_AH_DELAYCOMPENSATION_H

// \file 
// Application holder for the CS1 delay compensation

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <CEPFrame/ApplicationHolder.h>
#include <CS1_Interface/CS1_Parset.h>

namespace LOFAR 
{
  namespace CS1
  {
    //# Forward Declarations
    class Stub_Delay;
  
    // \addtogroup CS1_DelayCompensation
    // @{

    // Description of class.
    class AH_DelayCompensation : public ApplicationHolder
    {
    public:
      AH_DelayCompensation();
      virtual ~AH_DelayCompensation();

      virtual void define (const KeyValueMap&);
      virtual void run (int nsteps);

    private:
      // Copying is not allowed
      AH_DelayCompensation (const AH_DelayCompensation& that);
      AH_DelayCompensation& operator= (const AH_DelayCompensation& that);

      //# Datamembers
      CS1_Parset *itsCS1PS;
      // Stub for the DH_Delay data holder
      Stub_Delay* itsStub;

      //  Allocate a tracer context
      ALLOC_TRACER_CONTEXT;

    };

    // @}

  } // namespace CS1

} // namespace LOFAR

#endif
