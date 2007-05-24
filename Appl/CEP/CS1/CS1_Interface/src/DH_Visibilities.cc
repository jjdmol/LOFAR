//#  DH_Visibilities.cc:
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

#include <CS1_Interface/DH_Visibilities.h>
#include <Common/Timer.h>

namespace LOFAR {
namespace CS1 {

DH_Visibilities::DH_Visibilities(const string &name, const CS1_Parset *pSet)
: DataHolder(name, "DH_Visibilities"),
  itsCS1PS  (pSet),
  itsVisibilities(0),
  itsNrValidSamples(0)
{
  itsNrChannels       = itsCS1PS->nrChannelsPerSubband();
  unsigned nrStations = itsCS1PS->nrStations();
  itsNrBaselines      = nrStations * (nrStations + 1) / 2;
}   


DH_Visibilities::DH_Visibilities(const DH_Visibilities &that)
: DataHolder(that),
  itsCS1PS(that.itsCS1PS),
  itsNrBaselines(that.itsNrBaselines),
  itsNrChannels(that.itsNrChannels),
  itsVisibilities(0),
  itsNrValidSamples(0)
{
}

DH_Visibilities::~DH_Visibilities()
{
}

DataHolder* DH_Visibilities::clone() const
{
  return new DH_Visibilities(*this);
}

void DH_Visibilities::init()
{
  addField("Visibilities",   BlobField<fcomplex>(1, getNrVisibilities()), 32);
  addField("NrValidSamples", BlobField<NrValidSamplesType>(1, itsNrBaselines * itsNrChannels));

  createDataBlock();  // calls fillDataPointers
}

DH_Visibilities &DH_Visibilities::operator += (const DH_Visibilities &dh)
{
  NSTimer timer("DH_Vis add", true);
  timer.start();

  for (unsigned i = 0; i < getNrVisibilities(); i ++)
    itsVisibilities[i] += dh.itsVisibilities[i];

  for (unsigned i = 0; i < itsNrBaselines * itsNrChannels; i ++)
    itsNrValidSamples[i] += dh.itsNrValidSamples[i];

  timer.stop();
  return *this;
}

void DH_Visibilities::fillDataPointers() 
{
  itsVisibilities   = (VisibilityType *)     getData<fcomplex>("Visibilities");
  itsNrValidSamples = (NrValidSamplesType *) getData<NrValidSamplesType>("NrValidSamples");
}

} // namespace CS1
} // namespace LOFAR
