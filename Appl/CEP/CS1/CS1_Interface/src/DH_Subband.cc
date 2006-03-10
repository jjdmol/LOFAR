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
  itsFlags(0),
  itsMatrix(0)
{
}

DH_Subband::DH_Subband(const DH_Subband &that)
: DataHolder(that),
  itsSamples(that.itsSamples),
  itsFlags(that.itsFlags),
  itsMatrix(that.itsMatrix)
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
}


void DH_Subband::swapBytes()
{
  dataConvert(LittleEndian, (i16complex *) itsSamples, nrSamples());
}


void DH_Subband::setTestPattern(double Hz)
{
  // Inject a monochrome complex signal into the PPF; with the Y polarization
  // of station 1 75 cm apart to introduce a delay.  Also, a few samples are
  // flagged.

  static NSTimer timer("setTestPattern", true);
  timer.start();
  (std::cerr << "DH_Subband::setTestPattern() ... ").flush();

#if 1
  const double c = 299792458;
  const double antennaDistance = 5.99195103633380268132 * .25; // meter

  double labda = c / Hz;
  double phaseShift = 2 * M_PI * antennaDistance / labda;

  for (int time = 0; time < NR_INPUT_SAMPLES; time ++) {
    double s, c, phi = 2 * M_PI * Hz * time / SAMPLE_RATE;
    sincos(phi, &s, &c);
    i16complex sample = makei16complex((int) (32767 * c), (int) (32767 * s));

    for (int stat = 0; stat < NR_STATIONS; stat ++) {
      for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	(*itsSamples)[stat][time][pol] = sample;
      }
    }
#if NR_STATIONS >= 2 && NR_POLARIZATIONS == 2
    sincos(phi + phaseShift, &s, &c);
    (*itsSamples)[1][time][1] = makei16complex((int) (32767 * c), (int) (32767 * s));
#endif
  }
#else // use random samples
  for (size_t i = 0; i < nrSamples(); i ++) {
    ((SampleType *) itsSamples)[i] = makei16complex(rand() << 20 >> 20, rand() << 20 >> 20);
  }
#endif

  memset(itsFlags, 0, sizeof *itsFlags);

#if 0 && NR_INPUT_SAMPLES >= 17000
  (*itsFlags)[4][14000] = true;
  (*itsFlags)[5][17000] = true;
#endif

  (std::cerr << "done.\n").flush();

#if defined WORDS_BIGENDIAN
  (std::cerr << "swapBytes()\n").flush();
  swapBytes();
#endif

  timer.stop();
}

}
