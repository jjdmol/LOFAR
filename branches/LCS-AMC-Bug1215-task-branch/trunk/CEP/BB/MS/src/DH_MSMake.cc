//#  DH_MSMake.cc: DataHolder for the MS info
//#
//#  Copyright (C) 2005
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

#include <MS/DH_MSMake.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>

#include <casa/Arrays/Array.h>

#include <iostream>
#include <sstream>

using namespace casa;


namespace LOFAR {

  DH_MSMake::DH_MSMake (const string& name)
    : DataHolder (name, "DH_MSMake", 1)
  {
    setExtraBlob ("Extra", 1);
  }

  DH_MSMake::DH_MSMake (const DH_MSMake& that)
    : DataHolder (that)
  {}

  DH_MSMake::~DH_MSMake()
  {}

  DH_MSMake* DH_MSMake::clone() const
  {
    return new DH_MSMake (*this);
  }

  void DH_MSMake::init()
  {
    // Initialize the fieldset.
    initDataFields();
    // Add the fields to the data definition.
    addField ("NBand", BlobField<int>(1));
    addField ("NFreq", BlobField<int>(1));
    addField ("NTime", BlobField<int>(1));
    addField ("TileSizeFreq", BlobField<int>(1));
    addField ("TileSizeRest", BlobField<int>(1));
    addField ("Freqs", BlobField<double>(1, 2));
    addField ("Times", BlobField<double>(1, 2));
    // Create the data blob (which calls fillDataPointers).
    createDataBlock();
    // Set default tile sizes.
    *itsTileSizeFreq = -1;
    *itsTileSizeRest = -1;
  }

  void DH_MSMake::fillDataPointers()
  {
    // Fill in the pointers.
    itsNBand = getData<int> ("NBand");
    itsNFreq = getData<int> ("NFreq");
    itsNTime = getData<int> ("NTime");
    itsTileSizeFreq = getData<int> ("TileSizeFreq");
    itsTileSizeRest = getData<int> ("TileSizeRest");
    itsFreqs = getData<double> ("Freqs");
    itsTimes = getData<double> ("Times");
  }

  void DH_MSMake::fillExtra (const string& msName,
			     const Array<double>& ra,
			     const Array<double>& dec,
			     const Array<double>& antPos,
			     const Array<String>& antNames,
			     bool writeAutoCorr)
  {
    createExtraBlob() << msName << ra << dec
		      << antPos << antNames << writeAutoCorr;
  }

  void DH_MSMake::getExtra (string& msName,
			    Array<double>& ra,
			    Array<double>& dec,
			    Array<double>& antPos,
			    Array<String>& antNames,
			    bool& writeAutoCorr)
  {
    BlobIStream& bis = getExtraBlob();
    bis >> msName >> ra >> dec
	>> antPos >> antNames >> writeAutoCorr;
    bis.getEnd();
  }




  MSMakeConn::MSMakeConn (int nslave)
    : sender   ("msinfo_s"),
      receiver ("msinfo_r")
  {
#ifdef HAVE_MPI
    ths.resize (nslave);
    conns.resize (nslave);
    for (int i=0; i<nslave; ++i) {
      std::ostringstream str;
      str << i+1;
      ths[i] = new TH_MPI(0,i+1);
      conns[i] = new Connection("connect"+str.str(), &sender, &receiver,
				ths[i], true);
    }
#endif
    sender.init();
    receiver.init();
  }

  MSMakeConn::~MSMakeConn()
  {
#ifdef HAVE_MPI
    for (uint i=0; i<conns.size(); ++i) {
      delete conns[i];
      delete ths[i];
    }
#endif
  }
}
