//#  DH_RSP.cc: DataHolder storing RSP raw ethernet frames for 
//#             StationCorrelator demo
//#
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

#include <CS1_Interface/DH_RSP.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{
  namespace CS1
  {

    DH_RSP::DH_RSP (const string& name,
                    const ACC::APS::ParameterSet &pset)
      : DataHolder (name, "DH_RSP"),
        itsBuffer  (0),
	itsFlags   (0),
        itsPSet    (pset)
    {
      setExtraBlob("Flags", 0);

      int resendAmount = (pset.getInt32("BGLProc.NPPFTaps") - 1) * pset.getInt32("Observation.NChannels");
      itsNTimes          = pset.getInt32("Observation.NSubbandSamples") + resendAmount;
      itsNoPolarisations = pset.getInt32("Observation.NPolarisations");
      itsNSubbands       = pset.getInt32("Observation.NSubbandsPerCell");
      itsBufSize         = itsNTimes * itsNoPolarisations * itsNSubbands;
    }

    DH_RSP::DH_RSP(const DH_RSP& that)
      : DataHolder         (that),
        itsBuffer          (0),
	itsFlags	   (that.itsFlags),
        itsNTimes          (that.itsNTimes),
        itsNoPolarisations (that.itsNoPolarisations),
	itsNSubbands       (that.itsNSubbands),
        itsBufSize         (that.itsBufSize),
        itsPSet            (that.itsPSet)
    {}

    DH_RSP::~DH_RSP()
    {
      delete itsFlags;
    }

    DataHolder* DH_RSP::clone() const
    {
      return new DH_RSP(*this);
    }

    void DH_RSP::init()
    {
      // Add the fields to the data definition.
      addField ("Buffer", BlobField<BufferType>(1,itsBufSize));
      addField ("StationID", BlobField<int>(1));
      addField ("Delay", BlobField<int>(1));
      addField ("TimeStamp", BlobField<char>(1, sizeof(timestamp_t)));
  
      vector<DimDef> vdd;
      // there is one station per dataholder
      vdd.push_back(DimDef("Stations", 1));
      vdd.push_back(DimDef("Times", itsNTimes));
      vdd.push_back(DimDef("Subbands", itsNSubbands));
      vdd.push_back(DimDef("Polarisations", itsNoPolarisations));
  
      itsMatrix = new RectMatrix<BufferType> (vdd);
      itsFlags  = new SparseSet;

      // Create the data blob
      createDataBlock();
    }

    void DH_RSP::fillDataPointers()
    {
      // Fill in the buffer pointer.
      itsBuffer  = getData<BufferType> ("Buffer");
      itsMatrix->setBuffer(itsBuffer, itsBufSize);

      // Fill in the StationID pointer
      itsStationID = getData<int> ("StationID");
  
      // Fill in the Delay pointer
      itsDelay = getData<int> ("Delay");
  
      // Fill in TimeStamp pointer
      itsTimeStamp = (timestamp_t*)getData<char> ("TimeStamp");

      // use memset to null the buffer
      memset(itsBuffer, 0, itsBufSize*sizeof(BufferType));
    }

    void DH_RSP::fillExtraData()
    {
      itsFlags->write(createExtraBlob());
    }

    void DH_RSP::getExtraData()
    {
      itsFlags->read(getExtraBlob());
    }

  } // namespace CS1

} // end namespace LOFAR
