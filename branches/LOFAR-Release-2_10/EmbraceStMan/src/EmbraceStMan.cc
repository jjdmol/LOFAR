//# EmbraceStMan.cc: Storage Manager for the main table of an EMBRACE MS
//# Copyright (C) 2012
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

#include <lofar_config.h>
#include <EmbraceStMan/EmbraceStMan.h>
#include <EmbraceStMan/EmbraceColumn.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/DataManError.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/BlockIO.h>
#include <casa/IO/AipsIO.h>
#include <casa/OS/CanonicalConversion.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/DOos.h>
#include <casa/Utilities/Assert.h>
#include <casa/iostream.h>

using namespace casa;


namespace EMBRACE {

EmbraceStMan::EmbraceStMan (const String& dataManName)
: DataManager    (),
  itsDataManName (dataManName),
  itsRegFile     (0)
{}

EmbraceStMan::EmbraceStMan (const String& dataManName,
                            const Record&)
: DataManager    (),
  itsDataManName (dataManName),
  itsRegFile     (0)
{}

EmbraceStMan::EmbraceStMan (const EmbraceStMan& that)
: DataManager    (),
  itsDataManName (that.itsDataManName),
  itsRegFile     (0)
{}

EmbraceStMan::~EmbraceStMan()
{
  for (uInt i=0; i<ncolumn(); i++) {
    delete itsColumns[i];
  }
  delete itsRegFile;
}

DataManager* EmbraceStMan::clone() const
{
  return new EmbraceStMan (*this);
}

String EmbraceStMan::dataManagerType() const
{
  return "EmbraceStMan";
}

String EmbraceStMan::dataManagerName() const
{
  return itsDataManName;
}

Record EmbraceStMan::dataManagerSpec() const
{
  return itsSpec;
}


DataManagerColumn* EmbraceStMan::makeScalarColumn (const String& name,
                                                   int dtype,
                                                   const String&)
{
  EmbraceColumn* col;
  if (name == "TIME"  ||  name == "TIME_CENTROID") {
    col = new TimeColumn(this, dtype);
  } else if (name == "ANTENNA1") {
    col = new Ant1Column(this, dtype);
  } else if (name == "ANTENNA2") {
    col = new Ant2Column(this, dtype);
  } else if (name == "INTERVAL"  ||  name == "EXPOSURE") {
    col = new IntervalColumn(this, dtype);
  } else if (name == "FLAG_ROW") {
    col = new FalseColumn(this, dtype);
  } else {
    col = new ZeroColumn(this, dtype);
  }
  itsColumns.push_back (col);
  return col;
}

DataManagerColumn* EmbraceStMan::makeDirArrColumn (const String& name,
                                                   int dataType,
                                                   const String& dataTypeId)
{
  return makeIndArrColumn (name, dataType, dataTypeId);
}

DataManagerColumn* EmbraceStMan::makeIndArrColumn (const String& name,
                                                   int dtype,
                                                   const String&)
{
  EmbraceColumn* col;
  if (name == "UVW") {
    col = new UvwColumn(this, dtype);
  } else if (name == "DATA") {
    col = new DataColumn(this, dtype);
  } else if (name == "FLAG") {
    col = new FlagColumn(this, dtype);
  } else if (name == "FLAG_CATEGORY") {
    col = new FlagCatColumn(this, dtype);
  } else if (name == "WEIGHT") {
    col = new WeightColumn(this, dtype);
  } else if (name == "SIGMA") {
    col = new SigmaColumn(this, dtype);
  } else if (name == "WEIGHT_SPECTRUM") {
    col = new WSpectrumColumn(this, dtype);
  } else {
    throw DataManError (name + " is unknown column for EmbraceStMan");
  }
  itsColumns.push_back (col);
  return col;
}

DataManager* EmbraceStMan::makeObject (const String& group, const Record& spec)
{
  // This function is called when reading a table back.
  return new EmbraceStMan (group, spec);
}

void EmbraceStMan::registerClass()
{
  DataManager::registerCtor ("EmbraceStMan", makeObject);
}

Bool EmbraceStMan::canAddRow() const
{
  return False;
}
Bool EmbraceStMan::canRemoveRow() const
{
  return False;
}
Bool EmbraceStMan::canAddColumn() const
{
  return True;
}
Bool EmbraceStMan::canRemoveColumn() const
{
  return True;
}

void EmbraceStMan::addRow (uInt)
{
  throw DataManError ("EmbraceStMan cannot add rows");
}
void EmbraceStMan::removeRow (uInt)
{
  throw DataManError ("EmbraceStMan cannot remove rows");
}
void EmbraceStMan::addColumn (DataManagerColumn*)
{}
void EmbraceStMan::removeColumn (DataManagerColumn*)
{}

Bool EmbraceStMan::flush (AipsIO&, Bool)
{
  return False;
}

void EmbraceStMan::create (uInt nrows)
{
  itsNrRows = nrows;
}

void EmbraceStMan::open (uInt, AipsIO&)
{
  throw DataManError ("EmbraceStMan::open should never be called");
}
uInt EmbraceStMan::open1 (uInt, AipsIO&)
{
  // Read meta info.
  init();
  openFile (table().isWritable());
  return itsNrRows;
}

void EmbraceStMan::prepare()
{
  for (uInt i=0; i<ncolumn(); i++) {
    itsColumns[i]->prepareCol();
  }
}

void EmbraceStMan::openFile (bool writable)
{
  // Open the data file using unbuffered IO.
  delete itsRegFile;
  itsRegFile = 0;
  itsRegFile = new LargeFiledesIO (LargeFiledesIO::open (itsFileName.c_str(),
                                                         writable));
  // Set correct number of rows.
  itsNrRows = itsRegFile->length() / itsBLDataSize;
  // Size the buffer if needed.
  if (Int64(itsBuffer.size()) < itsBLDataSize/4) {
    itsBuffer.resize (itsBLDataSize/4);
  }
}

void EmbraceStMan::resync (uInt)
{
  throw DataManError ("EmbraceStMan::resync should never be called");
}
uInt EmbraceStMan::resync1 (uInt)
{
  uInt nrows = itsRegFile->length() / itsBLDataSize;
  // Reopen file if different nr of rows.
  if (nrows != itsNrRows) {
    openFile (table().isWritable());
  }
  return itsNrRows;
}

void EmbraceStMan::reopenRW()
{
  openFile (true);
}

void EmbraceStMan::deleteManager()
{
  delete itsRegFile;
  itsRegFile = 0;
  DOos::remove (fileName()+"meta", False, False);
}

void EmbraceStMan::init()
{
  AipsIO aio(fileName() + "meta");
  itsVersion = aio.getstart ("EmbraceStMan");
  if (itsVersion > 1) {
    throw DataManError ("EmbraceStMan can only handle up to version 1");
  }
  Bool asBigEndian;
  aio >> itsAnt1;
  aio>> itsAnt2;
  aio >> itsStartTime;
  aio>> itsTimeIntv;
  aio>> itsNChan;
  aio >> itsNPol;
  aio>> asBigEndian;
  aio>> itsFileName;
  aio.getend();
  // Set start time to middle of first time slot.
  itsStartTime += itsTimeIntv*0.5;
  AlwaysAssert (itsAnt1.size() == itsAnt2.size(), AipsError);
  itsDoSwap  = (asBigEndian != HostInfo::bigEndian());
  // A block contains an float value per baseline,chan,pol.
  itsBLDataSize = itsNChan * itsNPol * 4;    // #bytes/baseline
  // Fill the specification record (only used for reporting purposes).
  itsSpec.define ("version", itsVersion);
  itsSpec.define ("bigEndian", asBigEndian);
  itsSpec.define ("fileName", itsFileName);
}

Double EmbraceStMan::time (uInt blocknr)
{
  return itsStartTime + blocknr*itsTimeIntv;
}

void EmbraceStMan::getData (uInt rownr, Complex* buf)
{
  itsRegFile->seek (rownr*itsBLDataSize);
  itsRegFile->read (itsBLDataSize, itsBuffer.storage());
  if (itsDoSwap) {
    for (int i=0; i<itsBLDataSize/4; ++i) {
      buf[i] = Complex();
      CanonicalConversion::reverse4 (buf+i, itsBuffer.storage()+i);
    }
  } else {
    for (int i=0; i<itsBLDataSize/4; ++i) {
      buf[i] = itsBuffer[i];
    }
  }
}

void EmbraceStMan::putData (uInt rownr, const Complex* buf)
{
  if (itsDoSwap) {
    for (int i=0; i<itsBLDataSize/4; ++i) {
      Float f = buf[i].real();
      CanonicalConversion::reverse4 (itsBuffer.storage()+i, &f);
    }
  } else {
    for (int i=0; i<itsBLDataSize/4; ++i) {
      itsBuffer[i] = buf[i].real();
    }
  }
  itsRegFile->seek (rownr*itsBLDataSize);
  itsRegFile->write (itsBLDataSize, itsBuffer.storage());
}

} //# end namespace
