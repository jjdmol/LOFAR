//# tBlobIO.cc: test program for the BlobIO class.
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCBase/BlobIO.h>
#include <AMCBase/ConverterCommand.h>
#include <AMCBase/ConverterStatus.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Position.h>
#include <AMCBase/Epoch.h>
#include <Blob/BlobIBufChar.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOBufChar.h>
#include <Blob/BlobOStream.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_typeinfo.h>

using namespace LOFAR;
using namespace LOFAR::AMC;


void test(const ConverterCommand& occ)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(ConverterCommand).name(), 1);
  bos << occ;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  ConverterCommand icc;
  bis.getStart(typeid(ConverterCommand).name());
  bis >> icc;
  bis.getEnd();

  ASSERTSTR(icc == occ, "icc = " << icc << "; occ = " << occ);
}


void test(const ConverterStatus& ocs)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(ConverterStatus).name(), 1);
  bos << ocs;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  ConverterStatus ics;
  bis.getStart(typeid(ConverterStatus).name());
  bis >> ics;
  bis.getEnd();

  ASSERTSTR(ics.get() == ocs.get() && 
            ics.text() == ocs.text(),
            "ics = " << ics << "; ocs = " << ocs);
}


void test(const Direction& od)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(Direction).name(), 1);
  bos << od;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  Direction id;
  bis.getStart(typeid(Direction).name());
  bis >> id;
  bis.getEnd();

  ASSERTSTR(id == od, "id = " << id << "; od = " << od);
}


void test(const Position& op)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(Position).name(), 1);
  bos << op;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  Position ip;
  bis.getStart(typeid(Position).name());
  bis >> ip;
  bis.getEnd();

  ASSERTSTR(ip == op, "ip = " << ip << "; op = " << op);
}


void test(const Epoch& oe)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);

  bos.putStart(typeid(Epoch).name(), 1);
  bos << oe;
  bos.putEnd();

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);

  Epoch ie;
  bis.getStart(typeid(Epoch).name());
  bis >> ie;
  bis.getEnd();

  // We must make the detour through the doubles dci and dco, otherwise the
  // comparison will be done directly inside the FPU (using 80 bits instead of
  // 64), which will cause the assert to fail.
  double dci(ie.mjd());
  double dco(oe.mjd());
  ASSERTSTR(dci == dco, "ie = " << ie << "; oe = " << oe);
}


int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    try { 
      test(ConverterCommand(ConverterCommand::INVALID)); 
    } catch (AssertError& e) {}
    test(ConverterCommand(ConverterCommand::J2000toAZEL));
    test(ConverterCommand(ConverterCommand::J2000toITRF));
    test(ConverterCommand(ConverterCommand::AZELtoJ2000));
    test(ConverterCommand(ConverterCommand::ITRFtoJ2000));
    try {
      // Force the use of an undefined enumerated value.
      test(ConverterCommand(static_cast<ConverterCommand::Commands>(1000)));
    } catch (AssertError& e) {}

    test(ConverterStatus());
    test(ConverterStatus(ConverterStatus::UNKNOWN, "Unknown error"));
    test(ConverterStatus(ConverterStatus::OK, "Success"));
    test(ConverterStatus(ConverterStatus::ERROR, "Converter error"));
    // Use of an undefined enumerated value result in UNKNOWN.
    test(ConverterStatus(static_cast<ConverterStatus::Status>(1000),
                         "Undefined error"));

    test(Direction());
    test(Direction(0.4, -0.19, Direction::ITRF));
    test(Direction(-1.2, 2.38, Direction::AZEL));

    test(Position());
    test(Position(0.25*M_PI, -0.33*M_PI, 1));
    test(Position(-0.67*M_PI, 0.75*M_PI, 249.98, Position::WGS84));

    test(Epoch());
    test(Epoch(0));

  } catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 0;
}
