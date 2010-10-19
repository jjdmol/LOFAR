//#  tBlobIO.cc: test program for the BlobIO class.
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCBase/AMCClient/BlobIO.h>
#include <AMCBase/AMCClient/ConverterCommand.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <Common/BlobIBufChar.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOBufChar.h>
#include <Common/BlobOStream.h>
#include <Common/LofarLogger.h>

#include <Common/lofar_iomanip.h>

using namespace LOFAR;
using namespace LOFAR::AMC;


void test(const ConverterCommand& cco)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(ConverterCommand).name(), 1);
  bos << cco;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  ConverterCommand cci;
  bis.getStart(typeid(ConverterCommand).name());
  bis >> cci;
  bis.getEnd();

  ASSERTSTR(cci.get() == cco.get(), 
            "cci = " << cci.get() << "; cco = " << cco.get());
}


void test(const SkyCoord& sco)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(SkyCoord).name(), 1);
  bos << sco;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  SkyCoord sci;
  bis.getStart(typeid(SkyCoord).name());
  bis >> sci;
  bis.getEnd();

  ASSERTSTR(sci.angle0() == sco.angle0() && 
            sci.angle1() == sco.angle1(), 
            "sci = " << sci << "; sco = " << sco);
}


void test(const EarthCoord& eco)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(EarthCoord).name(), 1);
  bos << eco;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  EarthCoord eci;
  bis.getStart(typeid(EarthCoord).name());
  bis >> eci;
  bis.getEnd();

  ASSERTSTR(eci.longitude() == eco.longitude() && 
            eci.latitude() == eco.latitude() &&
            eci.height() == eco.height(), 
            "eci = " << eci << "; eco = " << eco);
}


void test(const TimeCoord& tco)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(TimeCoord).name(), 1);
  bos << tco;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  TimeCoord tci;
  bis.getStart(typeid(TimeCoord).name());
  bis >> tci;
  bis.getEnd();

  // We must make the detour through the doubles dci and dco, otherwise the
  // comparison will be done directly inside the FPU (using 80 bits instead of
  // 64), which will cause the assert to fail.
  double dci(tci.mjd());
  double dco(tco.mjd());
  ASSERTSTR(dci == dco, "tci = " << tci << "; tco = " << tco);
}


int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    test(ConverterCommand(ConverterCommand::INVALID));
    test(ConverterCommand(ConverterCommand::AZELtoJ2000));
    test(ConverterCommand(ConverterCommand::J2000toAZEL));
    test(ConverterCommand(1000));

    test(SkyCoord());
    test(SkyCoord(0.4, -0.19));

    test(EarthCoord());
    test(EarthCoord(0.25*M_PI, -0.33*M_PI));
    test(EarthCoord(-0.67*M_PI, 0.75*M_PI, 249.98));

    test(TimeCoord());
    test(TimeCoord(0));

  } catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 0;
}
