//  DH_Subband.cc:
//
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

#include <DH_Subband.h>
#include <Common/DataConvert.h>
#include <Common/Timer.h>


namespace LOFAR
{

DH_Subband::DH_Subband(const string &name, const ACC::APS::ParameterSet &pSet)
: DataHolder(name, "DH_Subband"),
  itsSamples(0),
  itsMatrix(0),
  itsFlags(0),
  itsDelays(0)
{
}

DH_Subband::DH_Subband(const DH_Subband &that)
: DataHolder(that),
  itsSamples(that.itsSamples),
  itsMatrix(that.itsMatrix),
  itsFlags(that.itsFlags),
  itsDelays(that.itsDelays)
{
}

DH_Subband::~DH_Subband()
{
  delete itsMatrix;
}

DataHolder *DH_Subband::clone() const
{
  return new DH_Subband(*this);
}

void DH_Subband::init()
{
  addField("Samples", BlobField<uint8>(1, sizeof(SampleType) * nrSamples()), 32);
  addField("Flags",   BlobField<uint32>(1, sizeof(AllFlagsType) / sizeof(uint32)));
  addField("Delays",  BlobField<float>(1, sizeof(AllDelaysType) / sizeof(float)));

  createDataBlock();

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Station",      NR_STATIONS));
  vdd.push_back(DimDef("Time",	       NR_INPUT_SAMPLES));
  vdd.push_back(DimDef("Polarisation", NR_POLARIZATIONS));

  itsMatrix = new RectMatrix<SampleType> (vdd);
  itsMatrix->setBuffer((SampleType *) itsSamples, nrSamples());

  memset(itsFlags, 0, sizeof *itsFlags);
}

void DH_Subband::fillDataPointers()
{
  itsSamples = (AllSamplesType *) getData<uint8> ("Samples");
  itsFlags   = (AllFlagsType *)   getData<uint32>("Flags");
  itsDelays  = (AllDelaysType *)  getData<float> ("Delays");
}

void DH_Subband::swapBytes()
{
  // only convert Samples; CEPframe converts Flags and Delays
  dataConvert(LittleEndian, (SampleType *) itsSamples, nrSamples());
}

}
