//# LofarStMan.cc: Storage Manager for the main table of a LOFAR MS
//# Copyright (C) 2009
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

#include <LofarStMan/LofarStMan.h>
#include <LofarStMan/LofarColumn.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/DataManError.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/BlockIO.h>
#include <casa/IO/MMapIO.h>
#include <casa/IO/AipsIO.h>
#include <casa/OS/CanonicalConversion.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/DOos.h>
#include <casa/Utilities/Assert.h>
#include <casa/iostream.h>

using namespace casa;


namespace LOFAR {

LofarStMan::LofarStMan (const String& dataManName)
: DataManager    (),
  itsDataManName (dataManName),
  itsFile        (0), 
  itsSeqFile     (0)
{}

LofarStMan::LofarStMan (const String& dataManName,
                        const Record&)
: DataManager    (),
  itsDataManName (dataManName),
  itsFile        (0),
  itsSeqFile     (0)
{}

LofarStMan::LofarStMan (const LofarStMan& that)
: DataManager    (),
  itsDataManName (that.itsDataManName),
  itsFile        (0),
  itsSeqFile     (0)
{}

LofarStMan::~LofarStMan()
{
  for (uInt i=0; i<ncolumn(); i++) {
    delete itsColumns[i];
  }
  delete itsFile;
  if (itsSeqFile) delete itsSeqFile;
}

DataManager* LofarStMan::clone() const
{
  return new LofarStMan (*this);
}

String LofarStMan::dataManagerType() const
{
  return "LofarStMan";
}

String LofarStMan::dataManagerName() const
{
  return itsDataManName;
}

Record LofarStMan::dataManagerSpec() const
{
  return Record();
}


DataManagerColumn* LofarStMan::makeScalarColumn (const String& name,
                                                 int dtype,
                                                 const String&)
{
  LofarColumn* col;
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

DataManagerColumn* LofarStMan::makeDirArrColumn (const String& name,
                                                 int dataType,
                                                 const String& dataTypeId)
{
  return makeIndArrColumn (name, dataType, dataTypeId);
}

DataManagerColumn* LofarStMan::makeIndArrColumn (const String& name,
                                                 int dtype,
                                                 const String&)
{
  LofarColumn* col;
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
    throw DataManError (name + " is unknown column for LofarStMan");
  }
  itsColumns.push_back (col);
  return col;
}

DataManager* LofarStMan::makeObject (const String& group, const Record& spec)
{
  // This function is called when reading a table back.
  return new LofarStMan (group, spec);
}

void LofarStMan::registerClass()
{
  DataManager::registerCtor ("LofarStMan", makeObject);
}

Bool LofarStMan::canAddRow() const
{
  return False;
}
Bool LofarStMan::canRemoveRow() const
{
  return False;
}
Bool LofarStMan::canAddColumn() const
{
  return True;
}
Bool LofarStMan::canRemoveColumn() const
{
  return True;
}

void LofarStMan::addRow (uInt)
{
  throw DataManError ("LofarStMan cannot add rows");
}
void LofarStMan::removeRow (uInt)
{
  throw DataManError ("LofarStMan cannot remove rows");
}
void LofarStMan::addColumn (DataManagerColumn*)
{}
void LofarStMan::removeColumn (DataManagerColumn*)
{}

Bool LofarStMan::flush (AipsIO&, Bool)
{
  return False;
}

void LofarStMan::create (uInt nrows)
{
  itsNrRows = nrows;
}

void LofarStMan::open (uInt, AipsIO&)
{
  throw DataManError ("LofarStMan::open should never be called");
}
uInt LofarStMan::open1 (uInt, AipsIO&)
{
  // Read meta info.
  init();
  mapFile();
  return itsNrRows;
}

void LofarStMan::prepare()
{
  for (uInt i=0; i<ncolumn(); i++) {
    itsColumns[i]->prepareCol();
  }
}

void LofarStMan::mapFile()
{
  // Memory-map the data file.
  delete itsFile;
  itsFile = 0;
  if (table().isWritable()) {
    itsFile = new MMapIO (fileName() + "data", ByteIO::Update);
  } else {
    itsFile = new MMapIO (fileName() + "data");
  }
  // Set correct number of rows.
  itsNrRows = itsFile->getFileSize() / itsBlockSize * itsAnt1.size();

  delete itsSeqFile;
  itsSeqFile = 0;
  try {
    itsSeqFile = new MMapIO (fileName() + "seqnr");
  } catch (...) {
    delete itsSeqFile; 
    itsSeqFile = 0;
  }
  // check the size of the sequencenumber file, close file is it doesn't match
  // FIXME !!! Check this filesize
  if (itsSeqFile && (itsSeqFile->getFileSize() != itsNrRows * itsAnt1.size() * sizeof(uInt))) {
    delete itsSeqFile;
    itsSeqFile = 0;
  }
}

void LofarStMan::resync (uInt)
{
  throw DataManError ("LofarStMan::resync should never be called");
}
uInt LofarStMan::resync1 (uInt)
{
  uInt nrows = itsFile->getFileSize() / itsBlockSize * itsAnt1.size();
  // Remap file if different nr of rows.
  if (nrows != itsNrRows) {
    mapFile();
  }
  return itsNrRows;
}

void LofarStMan::reopenRW()
{
  delete itsFile;
  itsFile = 0;
  itsFile = new MMapIO (fileName() + "data", ByteIO::Update);
}

void LofarStMan::deleteManager()
{
  delete itsFile;
  itsFile = 0;
  DOos::remove (fileName()+"meta", False, False);
  DOos::remove (fileName()+"data", False, False);
}

void LofarStMan::init()
{
  AipsIO aio(fileName() + "meta");
  itsVersion = aio.getstart ("LofarStMan");
  if (itsVersion > 2) {
    throw DataManError ("LofarStMan can only handle up to version 2");
  }
  Bool asBigEndian;
  uInt alignment;
  aio >> itsAnt1 >> itsAnt2 >> itsStartTime >> itsTimeIntv >> itsNChan
      >> itsNPol >> itsMaxNrSample >> alignment >> asBigEndian;
  aio.getend();
  // Set start time to middle of first time slot.
  itsStartTime += itsTimeIntv*0.5;
  AlwaysAssert (itsAnt1.size() == itsAnt2.size(), AipsError);
  uInt nrant = itsAnt1.size();
  itsDoSwap  = (asBigEndian != HostInfo::bigEndian());
  // A block contains an Int seqnr, Complex data per baseline,chan,pol and
  // uShort nsample per baseline,chan. Align it as needed.
  itsBLDataSize = itsNChan * itsNPol * 8;    // #bytes/baseline
  if (alignment <= 1) {
    itsDataStart = 4;
    itsSampStart = itsDataStart + nrant*itsBLDataSize;
    switch (itsVersion) {
    case 1:
      itsBlockSize = itsSampStart + nrant*itsNChan*2;
      break;
    case 2:
      itsBlockSize = itsSampStart + nrant*4;
      break;
    default:
      throw DataManError("LofarStMan can only handle up to version 2");
    }
  } else {
    itsDataStart = alignment;
    itsSampStart = itsDataStart + (nrant*itsBLDataSize + alignment-1)
      / alignment * alignment;
    switch (itsVersion) {
    case 1:
      itsBlockSize = itsSampStart + (nrant*itsNChan*2 + alignment-1)
	/ alignment * alignment;
      break;
    case 2:
      itsBlockSize = itsSampStart + (nrant*4 + alignment-1)
	/ alignment * alignment;
      break;
    default:
      throw DataManError("LofarStMan can only handle up to version 2");
    }
  }
  if (itsDoSwap) {
    switch (itsVersion) {
    case 1:
      itsNSampleBuf.resize (itsNChan * 2);
      break;
    case 2:
      itsNSampleBufV2.resize(4);
      break;
    default:
      throw;
    }
  }
}

Double LofarStMan::time (uInt blocknr)
{
  Int seqnr;
  
  const void* ptr;
  /// FIXME !! init and check itsSeqFile for correct size
  if (itsSeqFile) {
    ptr = itsSeqFile->getReadPointer(blocknr * sizeof(uInt));
  } else {
    ptr = itsFile->getReadPointer (blocknr * itsBlockSize);
  }
  if (itsDoSwap) {
    CanonicalConversion::reverse4 (&seqnr, ptr);
  } else {
    seqnr = *static_cast<const Int*>(ptr);
  }
  return itsStartTime + seqnr*itsTimeIntv;
}

void LofarStMan::getData (uInt rownr, Complex* buf)
{
  uInt blocknr = rownr / itsAnt1.size();
  uInt baseline = rownr - blocknr*itsAnt1.size();
  uInt offset  = itsDataStart + baseline * itsBLDataSize;
  const void* ptr = itsFile->getReadPointer (blocknr * itsBlockSize + offset);
  if (itsDoSwap) {
    const char* from = (const char*)ptr;
    char* to = (char*)buf;
    const char* fromend = from + itsBLDataSize;
    while (from < fromend) {
      CanonicalConversion::reverse4 (to, from);
      to += 4;
      from += 4;
    }
  } else {
    memcpy (buf, ptr, itsBLDataSize);
  }
}

void LofarStMan::putData (uInt rownr, const Complex* buf)
{
  uInt blocknr = rownr / itsAnt1.size();
  uInt baseline = rownr - blocknr*itsAnt1.size();
  uInt offset  = itsDataStart + baseline * itsBLDataSize;
  void* ptr = itsFile->getWritePointer (blocknr * itsBlockSize + offset);
  if (itsDoSwap) {
    const char* from = (const char*)buf;
    char* to = (char*)ptr;
    const char* fromend = from + itsBLDataSize;
    while (from < fromend) {
      CanonicalConversion::reverse4 (to, from);
      to += 4;
      from += 4;
    }
  } else {
    memcpy (ptr, buf, itsBLDataSize);
  }
}

const uShort* LofarStMan::getNSample (uInt rownr, Bool swapIfNeeded)
{
  uInt blocknr = rownr / itsAnt1.size();
  uInt baseline = rownr - blocknr*itsAnt1.size();
  uInt offset  = itsSampStart + baseline * itsNChan*2;
  const void* ptr = itsFile->getReadPointer (blocknr * itsBlockSize + offset);
  const uShort* from = (const uShort*)ptr;
  if (!swapIfNeeded || !itsDoSwap) {
    return from;
  }
  uShort* to = itsNSampleBuf.storage();
  for (uInt i=0; i<itsNChan; ++i) {
    CanonicalConversion::reverse2 (to+i, from+i);
  }
  return to;
}

const uInt* LofarStMan::getNSampleV2 (uInt rownr, Bool swapIfNeeded)
{
  uInt blocknr = rownr / itsAnt1.size();
  uInt baseline = rownr - blocknr*itsAnt1.size();

  uInt offset  = itsSampStart + baseline * 4; 
  
  const void* ptr = itsFile->getReadPointer (blocknr * itsBlockSize + offset);
  const uInt* from = (const uInt*)ptr;

  if (!swapIfNeeded || !itsDoSwap) {
    return from;
  }

  uInt* to = itsNSampleBufV2.storage();
  CanonicalConversion::reverse4 (to, from);

  return to;
}

} //# end namespace
