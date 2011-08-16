//# DH_Request.h: DataHolder for a conversion request.
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_AMCBASE_DH_REQUEST_H
#define LOFAR_AMCBASE_DH_REQUEST_H

// \file
// DataHolder for a conversion request

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Transport/DataHolder.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace AMC
  {
    //# Forward declarations
    class ConverterCommand;
    struct RequestData;

    // \addtogroup AMCBase
    // @{

    // This class contains the conversion request that will be sent from the
    // client to the server.
    class DH_Request : public DataHolder
    {
    public:

      // Constructor. We will use extra blobs to store our variable length
      // conversion requests, so we must initialize the blob machinery.
      DH_Request();

      // Destructor.
      virtual ~DH_Request();

      // Write the conversion request to be sent to the server into the I/O
      // buffers of the DataHolder.
      void writeBuf(const ConverterCommand&,
                    const RequestData&);
      
      // Read the conversion request that was received from the client from
      // the I/O buffers of the DataHolder.
      void readBuf(ConverterCommand&,
                   RequestData&);
        
    private:

      // Make a deep copy.
      // \note Must be redefined, because it's defined pure virtual in the
      // base class. Made it private, because we won't use it.
      virtual DH_Request* clone() const;

      // Version number for this class
      static const int theirVersionNr;
     
    };

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
