//  DH_Visibilities.cc:
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

#include <APS/ParameterSet.h>

#include <DH_Visibilities.h>

namespace LOFAR
{

DH_Visibilities::DH_Visibilities (const string& name, const ACC::APS::ParameterSet &pSet)
: DataHolder     (name, "DH_Visibilities"),
  //itsPS         (pSet),
  itsVisibilities(0)
{
#if 0
  //todo: support for multiple freq channels
   itsNPols = itsPS.getInt32("Data.NPolarisations");
   itsNCorrs = itsNPols*itsNPols;
   itsNStations  = itsPS.getInt32("FakeData.NStations");
   itsNBaselines = itsNStations * (itsNStations + 1)/2;
#endif
}   


DH_Visibilities::DH_Visibilities(const DH_Visibilities& that)
  : DataHolder    (that),
    //itsPS         (that.itsPS),
    itsVisibilities     (0)
#if 0
    itsNStations  (that.itsNStations),
    itsNBaselines (that.itsNBaselines),
    itsNPols      (that.itsNPols),
    itsNCorrs     (that.itsNCorrs)
#endif
{}

DH_Visibilities::~DH_Visibilities()
{}

DataHolder* DH_Visibilities::clone() const
{
  return new DH_Visibilities(*this);
}

void DH_Visibilities::init()
{
  addField("Visibilities", BlobField<fcomplex>(1, getBufSize()), 32);
  addField("NrValidSamplesCounted", BlobField<CountType>(1, NR_BASELINES * NR_SUBBAND_CHANNELS));

  createDataBlock();  // calls fillDataPointers
}

void DH_Visibilities::fillDataPointers() 
{
  itsVisibilities = (VisibilitiesType *)getData<fcomplex> ("Visibilities");
  itsNrValidSamplesCounted = (NrValidSamplesType *)getData<CountType> ("NrValidSamplesCounted");
}

void DH_Visibilities::checkCorrelatorTestPattern()
{
#if 0
  float sum = 0.0;

  for (int ch = 0; ch < NR_SUBBAND_CHANNELS; ch ++)
    sum += real((*itsVisibilities)[0][ch][0][0]);

  std::cout << sum << '\n';
#endif
  //std::cout << (*itsVisibilities)[0][73][0][0] << ' ' << (*itsVisibilities)[0][74][0][0] << '\n';
#if 1
  static const int channels[] = { 0, 73, 255 };

  for (int stat1 = 0; stat1 < std::min(NR_STATIONS, 8); stat1 ++) {
    for (int stat2 = stat1; stat2 < std::min(NR_STATIONS, 8); stat2 ++) {
      std::cout << "S(" << stat1 << ") * ~S(" << stat2 << ") :\n";
      int bl = baseline(stat1, stat2);
      for (int pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	for (int pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	  std::cout << " " << (char) ('x' + pol1) << (char) ('x' + pol2) << ':';
	  for (int chidx = 0; chidx < sizeof(channels) / sizeof(int); chidx ++) {
	    int ch = channels[chidx];
	    if (ch < NR_SUBBAND_CHANNELS) {
	      std::cout << ' ' << (*itsVisibilities)[bl][ch][pol1][pol2] << '/' << (*itsNrValidSamplesCounted)[bl][ch];
	    }
	  }
	  std::cout << '\n';
	}
      }
    }
  }

  std::cout << "newgraph yaxis newcurve linetype solid marktype none pts\n";
  float max = 0.0;

  for (int ch = 0; ch < NR_SUBBAND_CHANNELS; ch ++)
    if (real((*itsVisibilities)[0][ch][0][0]) > max)
      max = real((*itsVisibilities)[0][ch][0][0]);

  for (int ch = 0; ch < NR_SUBBAND_CHANNELS; ch ++)
    std::cout << ch << ' ' << (10 * std::log10(real((*itsVisibilities)[0][ch][0][0]) / max)) << '\n';
#endif
}

void DH_Visibilities::setStorageTestPattern(int factor)
{
  for (int bl = 0; bl < NR_BASELINES; bl++)
  {
    for (int ch = 0; ch < NR_SUBBAND_CHANNELS; ch++)
    {
      // Set number of valid samples
      (*itsNrValidSamplesCounted)[bl][ch] = bl*ch;

      for (int pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++)
      {
	for (int pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++)
	{
	  // Set visibilities
	  (*itsVisibilities)[bl][ch][pol1][pol2] = makefcomplex(bl+ch, factor*(pol1+pol2));
	}
      }
    }
  }
}

}
