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
                    const CS1_Parset *pSet)
      : DataHolder (name, "DH_RSP"),
        itsCS1PS   (pSet),
        itsBuffer  (0),
	itsFlags   (0)
    {
      setExtraBlob("Flags", 0);
      itsNTimes          = itsCS1PS->nrSamplesToBGLProc();
      itsNoPolarisations = itsCS1PS->getInt32("Observation.nrPolarisations");
      itsNSubbands       = itsCS1PS->nrSubbandsPerCell();
      itsBufSize         = itsNTimes * itsNoPolarisations * itsNSubbands;
    }

    DH_RSP::DH_RSP(const DH_RSP& that)
      : DataHolder         (that),
        itsCS1PS           (that.itsCS1PS),
        itsBuffer          (0),
	itsFlags	   (that.itsFlags),
        itsNTimes          (that.itsNTimes),
        itsNoPolarisations (that.itsNoPolarisations),
	itsNSubbands       (that.itsNSubbands),
        itsBufSize         (that.itsBufSize)
    {}

    DH_RSP::~DH_RSP()
    {
      delete itsFlags;
    };

    LOFAR::DataHolder* DH_RSP::clone() const
    {
      return new DH_RSP(*this);
    }

    void DH_RSP::init()
    {
      // Add the fields to the data definition.
      addField ("Buffer", BlobField<BufferType>(1,itsBufSize));
      addField ("StationID", BlobField<int>(1));
      addField ("Delay", BlobField<float>(1, 2));
      addField ("TimeStamp", BlobField<TimeStamp>(1));
  
      itsFlags = new SparseSet<unsigned>;

      // Create the data blob
      createDataBlock();
      // use memset to null the buffer
      memset(itsBuffer, 0, itsBufSize*sizeof(BufferType));
      itsFlags->write(createExtraBlob());
    }

    void DH_RSP::fillDataPointers()
    {
      // Fill in the buffer pointer.
      itsBuffer  = getData<BufferType> ("Buffer");

      // Fill in the StationID pointer
      itsStationID = getData<int> ("StationID");
  
      // Fill in the Delay pointer
      itsDelays = getData<float> ("Delay");
  
      // Fill in TimeStamp pointer
      itsTimeStamp = getData<TimeStamp> ("TimeStamp");
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
