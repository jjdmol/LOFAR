//#  tMeasurementSetFormat.cc: Test program for class MeasurementSetFormat
//#
//#  Copyright (C) 2011
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

#include <lofar_config.h>
#include <Storage/MeasurementSetFormat.h>

using namespace LOFAR;

int main()
{
  Parset parset("tMeasurementSetFormat.parset");
  MeasurementSetFormat msf(parset);
  msf.addSubband ("tMeasurementSetFormat_tmp.ms", 0, false);
}
