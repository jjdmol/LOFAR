//# DH_BBSStep.h: DataHolder for a BBSStep object.
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_DH_BBSSTEP_H
#define LOFAR_BBSCONTROL_DH_BBSSTEP_H

// \file
// Base component class of the DH_BBSStep composite pattern.

//# Includes
#include <BBSControl/BBSStep.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{
  //# Forward Declarations.

  namespace BBS
  {
    //# Forward Declarations.

    // \addtogroup BBS
    // @{

    // This class wraps a BBSStep object to prepare it for transport using the
    // %LOFAR Transport library.
    class DH_BBSStep : public DataHolder
    {
    public:
      // Constructor. We will use extra blobs to store our variable length
      // conversion requests, so we must initialize the blob machinery.
      DH_BBSStep();

      // Destructor.
      virtual ~DH_BBSStep();

      // Write the BBSStep to be sent into the I/O buffers of the DataHolder.
      void writeBuf(BBSStep*);
      
      // Read the BBSStep that was received from the I/O buffers of the
      // DataHolder.
      void readBuf(BBSStep*&);
        
    private:
      // Make a deep copy.
      // \note Must be redefined, because it's defined pure virtual in the
      // base class. Made it private, because we won't use it.
      virtual DH_BBSStep* clone() const;

      // Version number for this class
      static const int theirVersionNr;

    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
