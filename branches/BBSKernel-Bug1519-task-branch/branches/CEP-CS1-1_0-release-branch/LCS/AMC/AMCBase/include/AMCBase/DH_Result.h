//#  DH_Result.h: DataHolder for a conversion result.
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

#ifndef LOFAR_AMCBASE_DH_RESULT_H
#define LOFAR_AMCBASE_DH_RESULT_H

// \file
// DataHolder for a conversion result

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Transport/DataHolder.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace AMC
  {
    //# Forward declarations
    class ConverterStatus;
    struct ResultData;

    // \addtogroup AMCBase
    // @{

    // This class contains the conversion result that will be transferred from
    // the server to the client.
    class DH_Result : public DataHolder
    {
    public:
      // Constructor. We will use extra blobs to store our variable length
      // conversion results, so we must initialize the blob machinery.
      DH_Result();

      // Destructor.
      virtual ~DH_Result();

      // Write the conversion result to be sent to the client into the I/O
      // buffers of the DataHolder. 
      void writeBuf(const ConverterStatus&, const ResultData&);
      
      // Read the conversion result that was received from the server from the
      // I/O buffers of the DataHolder. 
      void readBuf(ConverterStatus&, ResultData&);

    private:
      // Make a deep copy.
      // \note Must be redefined, because it's defined pure virtual in the
      // base class. However, we won't implement it, because we won't use it.
      virtual DH_Result* clone() const;

      // Version number for this class
      static const int theirVersionNr;
     
    };

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
