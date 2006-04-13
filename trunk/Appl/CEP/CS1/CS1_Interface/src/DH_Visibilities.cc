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

DH_Visibilities::DH_Visibilities (const string &name, const ACC::APS::ParameterSet &pSet)
: DataHolder     (name, "DH_Visibilities"),
  //itsPS         (pSet),
  itsVisibilities(0),
  itsNrValidSamples(0)
{
#if 0
  //todo: support for multiple freq channels
   itsNPols = itsPS.getInt32("Observation.NPolarisations");
   itsNCorrs = itsNPols*itsNPols;
#endif
   itsNrChannels       = pSet.getUint32("Observation.NChannels");
   unsigned nrStations = pSet.getUint32("Observation.NStations");
   itsNrBaselines      = nrStations * (nrStations + 1) / 2;
}   


DH_Visibilities::DH_Visibilities(const DH_Visibilities &that)
  : DataHolder    (that),
    //itsPS         (that.itsPS),

    itsNrBaselines(that.itsNrBaselines),
    itsNrChannels(that.itsNrChannels),
    itsVisibilities(0),
    itsNrValidSamples(0)
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
  addField("Visibilities",   BlobField<fcomplex>(1, getNrVisibilities()), 32);
  addField("NrValidSamples", BlobField<NrValidSamplesType>(1, itsNrBaselines * itsNrChannels));

  createDataBlock();  // calls fillDataPointers
}

void DH_Visibilities::fillDataPointers() 
{
  itsVisibilities   = (VisibilityType *)     getData<fcomplex>("Visibilities");
  itsNrValidSamples = (NrValidSamplesType *) getData<NrValidSamplesType>("NrValidSamples");
}

}
