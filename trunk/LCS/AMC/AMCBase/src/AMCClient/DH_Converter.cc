//#  DH_Converter.cc: implementation of the Converter data holder class.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCBase/AMCClient/DH_Converter.h>
#include <AMCBase/AMCClient/BlobIO.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>

namespace LOFAR
{
  namespace AMC
  {

    // Initialize the verion number of this class.
    uint32 DH_Converter::theirVersionNr = 1;

    DH_Converter::DH_Converter() : 
      DataHolder("aDH_Converter", "DH_Converter", theirVersionNr)
    {
      // We will use extra blobs to store our variable length requests and
      // results. So we must initialize the blob machinery.
      setExtraBlob("Blob", theirVersionNr);
    }

    void DH_Converter::send(ConverterOperation oper,
                            const vector<SkyCoord>& sc,
                            const vector<EarthCoord>& ec,
                            const vector<TimeCoord>& tc)
    {
      // First we will create the output blob that will hold the request data
      // to be send to the converter server.
      BlobOStream& bos = createExtraBlob();

      // Fill the output blob with the request data.
      bos << theirVersionNr;
      bos << oper;
      bos << static_cast<uint32>(sc.size());
      for (uint i = 0; i < sc.size(); i++) {
        bos << sc[i];
      }
      bos << static_cast<uint32>(ec.size());
      for (uint i = 0; i < ec.size(); i++) {
        bos << ec[i];
      }
      bos << static_cast<uint32>(tc.size());
      for (uint i = 0; i < tc.size(); i++) {
        bos << tc[i];
      }

      // Send the blob to the converter server.
      write();
    }

    void DH_Converter::receive(vector<SkyCoord>& sc,
                               vector<EarthCoord>& ec,
                               vector<TimeCoord>& tc)
    {
      // Receive the blob from the converter server.
      read();

      // Open the input blob to read the data that were just received from the
      // converter server.
      BlobIStream& bis = getExtraBlob();

      // Fill the coordinate vectors with the input blob data, which hold the
      // result data from the converter server.
      {
        // Make sure we've received the correct version of this class.
        uint32 aVersionNr;
        bis >> aVersionNr;
        ASSERT(aVersionNr == theirVersionNr);

        // We won't use the operation field, since we're not a server.
        ConverterOperation anOper;
        bis >> anOper;
        
        uint32 len;

        // Get the size of the vector of sky coordinates and fill the vector.
        bis >> len;
        for (uint i = 0; i < static_cast<uint>(len); i++) {
          bis >> sc[i];
        }

        // Get the size of the vector of earth coordinates and fill the vector.
        bis >> len;
        for (uint i = 0; i < static_cast<uint>(len); i++) {
          bis >> ec[i];
        }

        // Get the size of the vector of time coordinates and fill the vector.
        bis >> len;
        for (uint i = 0; i < static_cast<uint>(len); i++) {
          bis >> tc[i];
        }
      }
    }


  } // namespace AMC

} // namespace LOFAR
