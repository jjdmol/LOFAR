//#  DH_Subband.cc:
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

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/DataConvert.h>
#include <Common/Timer.h>
#include <CS1_Interface/DH_Subband.h>


namespace LOFAR {
namespace CS1 {

DH_Subband::DH_Subband(const string &name, const ACC::APS::ParameterSet &pSet)
  : DataHolder(name, "DH_Subband"),
    itsNrStations(pSet.getUint32("Observation.NStations")),
    itsNrInputSamples(pSet.getUint32("Observation.NSubbandSamples") + (pSet.getUint32("BGLProc.NPPFTaps") - 1) * pSet.getUint32("Observation.NChannels")),
    itsSamples(0),
    itsSamplesMatrix(0),
    itsFlags(0),
    itsDelays(0)
{
  setExtraBlob("Flags", 0);

  ASSERT(pSet.getUint32("Observation.NSubbandSamples") % (pSet.getUint32("BGLProc.NPPFTaps") * pSet.getUint32("Observation.NChannels")) == 0);
}

DH_Subband::DH_Subband(const DH_Subband &that)
  : DataHolder(that),
    itsNrStations(that.itsNrStations),
    itsNrInputSamples(that.itsNrInputSamples),
    itsSamples(0),
    itsSamplesMatrix(0),
    itsFlags(0),
    itsDelays(0)
{
  setExtraBlob("Flags", 0);
}

DH_Subband::~DH_Subband()
{
  delete [] itsFlags;
  delete itsSamplesMatrix;
}

LOFAR::DataHolder *DH_Subband::clone() const
{
  return new DH_Subband(*this);
}

void DH_Subband::init()
{
  addField("Samples", BlobField<uint8>(1, nrSamples() * sizeof(SampleType)), 32);
  addField("Delays",  BlobField<float>(1, nrDelays() * sizeof(DelayIntervalType) / sizeof(float)));

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Station",      itsNrStations));
  vdd.push_back(DimDef("Time",	       itsNrInputSamples));
  vdd.push_back(DimDef("Polarisation", NR_POLARIZATIONS));

  itsSamplesMatrix = new RectMatrix<SampleType> (vdd);
  itsFlags	   = new SparseSet[itsNrStations];

  createDataBlock();
}

void DH_Subband::fillDataPointers()
{
  itsSamples = (SampleType *)	     getData<uint8> ("Samples");
  itsDelays  = (DelayIntervalType *) getData<float> ("Delays");

  itsSamplesMatrix->setBuffer(itsSamples, nrSamples());
}


void DH_Subband::fillExtraData()
{
  BlobOStream& bos = createExtraBlob();

  for (unsigned stat = 0; stat < itsNrStations; stat ++)
    itsFlags[stat].write(bos);
}
  

void DH_Subband::getExtraData()
{
  BlobIStream &bis = getExtraBlob();
  
  for (unsigned stat = 0; stat < itsNrStations; stat ++)
    itsFlags[stat].read(bis);
}


void DH_Subband::swapBytes()
{
  // only convert Samples; CEPframe converts Flags and Delays
  dataConvert(LittleEndian, itsSamples, nrSamples());
}

} // namespace CS1
} // namespace LOFAR
