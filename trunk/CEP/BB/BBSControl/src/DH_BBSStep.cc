//#  DH_BBSStep.cc: DataHolder for a BBSStep object.
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

#include <lofar_config.h>

#include <BBSControl/DH_BBSStep.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {
    // Initialize the verion number of this class.
    const int DH_BBSStep::theirVersionNr = 1;

    
    DH_BBSStep::DH_BBSStep() : 
      DataHolder("aDH_BBSStep", "DH_BBSStep", theirVersionNr)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // We will use extra blobs to store our variable length BBSSteps. So we
      // must initialize the blob machinery.
      setExtraBlob("DH_BBSStep", theirVersionNr);

      // We must call this method, even though we won't use the fixed buffer
      // that's being created. Neat, huh?
      createDataBlock();
    }


    DH_BBSStep::~DH_BBSStep()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    void DH_BBSStep::writeBuf(BBSStep* bs)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Create the output blob that will hold the BBSStep to be sent.
      BlobOStream& bos = createExtraBlob();

      // Fill the output blob with the BBSStep data. 
      // \note We don't need to call putStart() and putEnd() on the blob
      // stream; this is done by the DataBlobExtra class in the Transport
      // library.
      bs->serialize(bos);
    }


    void DH_BBSStep::readBuf(BBSStep*& bs)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Open the input blob to read the data that were just received.
      bool found;
      int version;
      BlobIStream& bis = getExtraBlob(found, version);

      // Make sure we've received the correct version of this class.
      ASSERT(found && version == theirVersionNr);
      
      // Retrieve the BBSStep data.
      bs = BBSStep::deserialize(bis);

      // Assert that we're really at the end of the blob.
      bis.getEnd();
    }


    DH_BBSStep* DH_BBSStep::clone() const
    {
      THROW(Exception, "We should NEVER call this method!");
    }


  } // namespace BBS

} // namespace LOFAR
