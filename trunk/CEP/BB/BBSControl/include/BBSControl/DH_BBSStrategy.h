//# DH_BBSStrategy.h: DataHolder for the parameters of a BBSStrategy.
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

#ifndef LOFAR_BBSCONTROL_DH_BBSSTRATEGY_H
#define LOFAR_BBSCONTROL_DH_BBSSTRATEGY_H

// \file
// Base component class of the DH_BBSStrategy composite pattern.

//# Includes
#include <Transport/DataHolder.h>

namespace LOFAR
{
  //# Forward Declarations.

  namespace BBS
  {
    //# Forward Declarations.
    class BBSStrategy;

    // \addtogroup BBS
    // @{

    // This class wraps all the data members of a BBSStrategy object, \e
    // except the BBSStep objects it contains, to prepare it for transport
    // using the %LOFAR Transport library.
    class DH_BBSStrategy : public DataHolder
    {
    public:
      // Constructor. We will use extra blobs to store our variable length
      // conversion requests, so we must initialize the blob machinery.
      DH_BBSStrategy();

      // Destructor.
      virtual ~DH_BBSStrategy();

      // Write all data members to be sent into the I/O buffers of the
      // DataHolder. The argument \a doSteps determines whether the BBSStep
      // objects within a BBSStrategy will be written as well. By default,
      // they won't.
      void writeBuf(const BBSStrategy& bs, bool doSteps = false);
      
      // Read all data members of the BBSStrategy object, \e except the
      // BBSStep objects, that were received from the I/O buffers of the
      // DataHolder.
      void readBuf(BBSStrategy& bs);
        
    private:
      // Make a deep copy.
      // \note Must be redefined, because it's defined pure virtual in the
      // base class. Made it private, because we won't use it.
      virtual DH_BBSStrategy* clone() const;

      // Version number for this class
      static const int theirVersionNr;

    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
