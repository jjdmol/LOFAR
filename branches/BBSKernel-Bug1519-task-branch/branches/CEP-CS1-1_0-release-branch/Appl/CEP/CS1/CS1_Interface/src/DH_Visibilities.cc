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


#if 0
// TODO: add nrValidSamples

extern "C" { void do_add(fcomplex *, const fcomplex *, unsigned); }

asm(
"do_add:\n"
"	.long 0x7c001b1c\n"	// lfpsx   0,0,3
"	li      8,8\n"
"	.long 0x7c20231c\n"	// lfpsx   1,0,4
"	addi    9,3,-8\n"
"	.long 0x7c43435c\n"	// lfpsux  2,3,8
"	rlwinm  5,5,30,2,31\n"
"	.long 0x7c64435c\n"	// lfpsux  3,4,8
"	mtctr   5\n"
"	.long 0x7c83435c\n"	// lfpsux  4,3,8
"	li	10,64\n"
"	.long 0x7ca4435c\n"	// lfpsux  5,4,8
"	.long 0x7cc3435c\n"	// lfpsux  6,3,8
"	.long 0x7ce4435c\n"	// lfpsux  7,4,8
"0:\n"
"	.long 0x01000818\n"	// fpadd   8,0,1
"	.long 0x7c03435c\n"	// lfpsux  0,3,8
"	.long 0x01221818\n"	// fpadd   9,2,3
"	.long 0x7c24435c\n"	// lfpsux  1,4,8
"	.long 0x01442818\n"	// fpadd   10,4,5
"	.long 0x7c43435c\n"	// lfpsux  2,3,8
"	.long 0x01663818\n"	// fpadd   11,6,7
"	.long 0x7c64435c\n"	// lfpsux  3,4,8
"	.long 0x7c83435c\n"	// lfpsux  4,3,8
"	.long 0x7d09475c\n"	// stfpsux 8,9,8
"	.long 0x7ca4435c\n"	// lfpsux  5,4,8
"	.long 0x7d29475c\n"	// stfpsux 9,9,8
"	.long 0x7cc3435c\n"	// lfpsux  6,3,8
"	.long 0x7d49475c\n"	// stfpsux 10,9,8
"	.long 0x7ce4435c\n"	// lfpsux  7,4,8
"	.long 0x7d69475c\n"	// stfpsux 11,9,8
"	bdnz+   0b\n"
"	blr\n"
);
#endif


DH_Visibilities &DH_Visibilities::operator += (const DH_Visibilities &dh)
{
  NSTimer timer("DH_Vis add", true);
  timer.start();

#if 1
  for (unsigned i = 0; i < getNrVisibilities(); i ++)
    itsVisibilities[i] += dh.itsVisibilities[i];

  for (unsigned i = 0; i < itsNrBaselines * itsNrChannels; i ++)
    itsNrValidSamples[i] += dh.itsNrValidSamples[i];
#else
  do_add(itsVisibilities, dh.itsVisibilities, getNrVisibilities());
#endif

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
