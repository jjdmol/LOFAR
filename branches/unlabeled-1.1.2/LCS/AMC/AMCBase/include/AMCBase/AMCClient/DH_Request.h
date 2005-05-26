//#  DH_Request.h: DataHolder for a conversion request.
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

#ifndef LOFAR_AMCBASE_AMCCLIENT_DH_REQUEST_H
#define LOFAR_AMCBASE_AMCCLIENT_DH_REQUEST_H

// \file DH_Request.h
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
    class SkyCoord;
    class EarthCoord;
    class TimeCoord;
    class ConverterCommand;

    // \addtogroup AMCClient
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
                    const vector<SkyCoord>&,
                    const vector<EarthCoord>&,
                    const vector<TimeCoord>&);
      
      // Read the conversion request that was received from the client from
      // the I/O buffers of the DataHolder.
      void readBuf(ConverterCommand&,
                   vector<SkyCoord>&,
                   vector<EarthCoord>&,
                   vector<TimeCoord>&);
        
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
