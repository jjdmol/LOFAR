//#  DH_RSP.h: This DataHolders holds the data a RSP-board will output
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

#ifndef LOFAR_GENERATOR_DH_RSP_H
#define LOFAR_GENERATOR_DH_RSP_H

// \file
// This DataHolders holds the data a RSP-board will output

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <APS/ParameterSet.h>
#include <Transport/DataHolder.h>
#include <Generator/RSPEthFrame.h>

namespace LOFAR 
{
  namespace Generator 
  {

    // \addtogroup Generator
    // @{

    //# Forward Declarations
    //class forward;


    // Description of class.
    class DH_RSP : public DataHolder
    {
    public:
      typedef u16complex DataType;

      explicit DH_RSP(const string& name,
		      const ACC::APS::ParameterSet& pset);

      DH_RSP(const DH_RSP&);

      virtual ~DH_RSP();

      DataHolder* clone() const;

      /// Allocate the buffers.
      virtual void init();

      // Access methods
      Frame* getFrame() const { return itsFrame;};
      
    private:
      // forbid assignment
      DH_RSP& operator= (const DH_RSP& that);

      // Fill the pointers (itsBuffer) to the data in the blob.
      virtual void fillDataPointers();
      //# Datamembers

      Frame* itsFrame;
      ACC::APS::ParameterSet itsPSet;
    };

    // @}

  } // namespace Generator
} // namespace LOFAR

#endif
