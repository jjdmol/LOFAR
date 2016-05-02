//# DH_Request.cc: implementation of the DH_Request data holder class.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCBase/DH_Request.h>
#include <AMCBase/BlobIO.h>
#include <AMCBase/ConverterCommand.h>
#include <AMCBase/RequestData.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobArray.h>
#include <Common/LofarLogger.h>


namespace LOFAR
{
  namespace AMC
  {

    // Initialize the verion number of this class.
    const int DH_Request::theirVersionNr = 3;

    DH_Request::DH_Request() : 
      DataHolder("aDH_Request", "DH_Request", theirVersionNr)
    {
      // We will use extra blobs to store our variable length requests and
      // results. So we must initialize the blob machinery.
      setExtraBlob("DH_Request", theirVersionNr);

      // We must call this method, even though we won't use the fixed buffer
      // that's being created. Neat, huh?
      createDataBlock();
    }


    DH_Request::~DH_Request()
    {
    }


    void DH_Request::writeBuf(const ConverterCommand& command,
                              const RequestData& request)
    {
      // Create the output blob that will hold the request data to be sent to
      // the converter server.
      BlobOStream& bos = createExtraBlob();

      // Fill the output blob with the request data. 
      // \note We don't need to call putStart() and putEnd() on the blob
      // stream; this is done by the DataBlobExtra class in the Transport
      // library.
      bos << command 
          << request;
    }


    void DH_Request::readBuf(ConverterCommand& command,
                             RequestData& request)
    {
      // Open the input blob to read the data that were just received from the
      // converter client.
      bool found;
      int version;
      BlobIStream& bis = getExtraBlob(found, version);

      // Make sure we've received the correct version of this class.
      ASSERT(found && version == theirVersionNr);
      
      // Retrieve the converter request.
      bis >> command
          >> request;

      // Assert that we're really at the end of the blob.
      bis.getEnd();
    }


    DH_Request* DH_Request::clone() const
    {
      THROW(Exception, "We should NEVER call this method!");
    }

  } // namespace AMC

} // namespace LOFAR
