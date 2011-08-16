//#  tMeasurementSetFormat.cc: Test program for class MeasurementSetFormat
//#
//#  Copyright (C) 2011
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

#include <lofar_config.h>
#include <Storage/MeasurementSetFormat.h>
#include <Common/LofarLogger.h>

#include <casa/IO/RegularFileIO.h>

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace casa;

// Define handler that tries to print a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main()
{
  try {
    Parset parset("tMeasurementSetFormat.parset");
    MeasurementSetFormat msf(parset);
    msf.addSubband("tMeasurementSetFormat_tmp.ms", 0, false);
    // Also create the data file, otherwise it is not a true table.
    ///FILE* file= fopen ("tMeasurementSetFormat_tmp.ms/f0data", "w");
    ///fclose (file);
    RegularFileIO file(String("tMeasurementSetFormat_tmp.ms/table.f0data"),
    		       ByteIO::New);
  } catch (LOFAR::Exception &err) {
    std::cerr << "LOFAR Exception detected: " << err << std::endl;
    return 1;
  }
  return 0;
}
