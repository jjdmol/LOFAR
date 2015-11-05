//# parmexportcal.cc: Export calibration solutions
//#
//# Copyright (C) 2011
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

#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmFacade.h>
#include <ParmDB/ParmMap.h>
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/ParmValue.h>
#include <ParmDB/Package__Version.h>

#include <Common/InputParSet.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>
#include <Common/Exception.h>

#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/MUString.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>
#include <pwd.h>

using namespace casa;
using namespace LOFAR;
using namespace BBS;

// Define handler that tries to print a backtrace.
Exception::TerminateHandler t(Exception::terminate);

enum parmValType {KEEP, POLAR, COMPLEX};

// Create a new parm for export.
void writeAsValue (const String& parmName, const Matrix<double>& values,
                   ParmDB& parmdb,
                   const Vector<double>& freqc, const Vector<double>& freqw)
{
  // There should be one time slot.
  ASSERT (values.shape()[1] == 1);
  // Form the grid.
  Axis::ShPtr timeAxis(new RegularAxis(-1e30, 1e30, 1, true));
  Axis::ShPtr freqAxis;
  if (allNear (freqw, freqw[0], 1e-5)) {
    freqAxis = Axis::ShPtr(new RegularAxis(freqc[0]-freqw[0]*0.5,
                                           freqw[0], freqc.size()));
  } else {
    Vector<double> freqs(freqc - freqw*0.5);
    vector<double> fr(freqs.begin(), freqs.end());
    vector<double> fw(freqw.begin(), freqw.end());
    freqAxis = Axis::ShPtr(new OrderedAxis(fr, fw));
  }
  Grid grid (freqAxis, timeAxis);
  // Create the parm value.
  ParmValue::ShPtr pval(new ParmValue);
  pval->setScalars (grid, values);
  // Create the ParmValueSet.
  vector<ParmValue::ShPtr> valvec (1, pval);
  vector<Box> domains(1, grid.getBoundingBox());
  // Use absolute perturbation for phases.
  String pname = parmName;
  pname.gsub (":Phase:", "");
  bool pertrel = (parmName == pname);
  ParmValueSet pvset(Grid(domains), valvec, ParmValue(),
                     ParmValue::Scalar, 1e-6, pertrel);
  // Add the parameter name to the ParmDB (it sets the id).
  int nameId = -1;
  parmdb.putValues (parmName, nameId, pvset);
  LOG_INFO_STR ("Wrote new record for parameter " << parmName);
}

void writeAsDefault (const String& parmName, double value,
                     ParmDB& parmdb)
{
  ParmValue pval(value);
  ParmValue::FunkletType type = ParmValue::Scalar;
  // Get perturbation for numerical derivatives.
  double pert = 1e-6;
  // Use absolute perturbation for phases.
  String pname = parmName;
  pname.gsub (":Phase:", "");
  bool pertrel = (parmName == pname);
  ParmValueSet pvset(pval, type, pert, pertrel);
  parmdb.putDefValue (parmName, pvset);
  LOG_INFO_STR ("Wrote new defaultvalue record for parameter " << parmName);
}

void writeParm (const Matrix<double>& val, const String& name,
                ParmDB& parmdb,
                const Vector<double>& freqs, const Vector<double>& freqw)
{
  if (val.size() == 1) {
    writeAsDefault (name, *val.data(), parmdb);
  } else {
    writeAsValue (name, val, parmdb, freqs, freqw);
  }
}

Matrix<double> getAP (const Matrix<double>& ampl,
                      const Matrix<double>& phase,
                      bool phase0,
                      double perc, long skipLast)
{
  // Get the median in time for each frequency.
  ASSERT (ampl.shape() == phase.shape());
  const IPosition& shp = ampl.shape();
  int nf = shp[0];
  // Optionally ignore the last value.
  int nt = std::max (1L, shp[1] - skipLast);
  Matrix<double> result(nf,2);
  for (int i=0; i<nf; ++i) {
    Matrix<double> aline = ampl(IPosition(2,i,0), IPosition(2,i,nt-1));
    double med = median(aline);
    if (abs(aline(i,nt-1) - med) > perc/100*med) {
      LOG_INFO_STR ("amplitude " << aline(i,nt-1) << " differs more than "
                    << perc << "% from median " << med);
    }
    result(i,0) = med;
    result(i,1) = (phase0  ?  0 : phase(i, nt-1));
  }
  return result;
}

void processRI (const Record& realValues, const Record& imagValues,
                double perc, int skipLast, bool phase0,
                ParmDB& parmdb, parmValType type)
{
  for (uInt i=0; i<realValues.nfields(); ++i) {
    String name = realValues.name(i);
    // Replace real by imaginary to find its values.
    name.gsub (":Real:", ":Imag:");
    const RecordInterface& reals = realValues.asRecord(i);
    const RecordInterface& imags = imagValues.asRecord(name);
    Matrix<double> real (reals.asArrayDouble("values"));
    Matrix<double> imag (imags.asArrayDouble("values"));
    int nf = real.shape()[0];
    // Get the median ampl and last phase per frequency.
    Matrix<double> ap (getAP (sqrt(real*real + imag*imag),
                              atan2(real, imag),
                              phase0, perc, skipLast));
    // Write the results as real/imag or ampl/phase.
    Matrix<double> medAmpl   (ap(IPosition(2,0,0), IPosition(2,nf-1,0)));
    Matrix<double> lastPhase (ap(IPosition(2,0,1), IPosition(2,nf-1,1)));
    Vector<Double> freqs (reals.asArrayDouble("freqs"));
    Vector<Double> freqw (reals.asArrayDouble("freqwidths"));
    switch (type) {
    case KEEP:
    case COMPLEX:
      writeParm (medAmpl*sin(lastPhase), realValues.name(i), parmdb,
                 freqs, freqw);
      writeParm (medAmpl*cos(lastPhase), name, parmdb, freqs, freqw);
      break;
    case POLAR:
      name.gsub (":Imag:", ":Ampl:");
      writeParm (medAmpl, name, parmdb, freqs, freqw);
      name.gsub (":Ampl:", ":Phase:");
      writeParm (lastPhase, name, parmdb, freqs, freqw);
      break;
    }
  }
}

void processAP (const Record& amplValues, const Record& phaseValues,
                double perc, int skipLast, bool phase0,
                ParmDB& parmdb, parmValType type)
{
  for (uInt i=0; i<amplValues.nfields(); ++i) {
    String name = amplValues.name(i);
    // Replace amplitude by phase to find its values.
    name.gsub (":Ampl:", ":Phase:");
    const RecordInterface& ampls  = amplValues.asRecord(i);
    const RecordInterface& phases = phaseValues.asRecord(name);
    Matrix<double> ampl  (ampls.asArrayDouble("values"));
    Matrix<double> phase (phases.asArrayDouble("values"));
    int nf = ampl.shape()[0];
    // Get the median ampl and last phase per frequency.
    Matrix<double> ap (getAP (ampl, phase, phase0, perc, skipLast));
    // Write the results as ampl/phase or real/imag.
    Matrix<double> medAmpl   (ap(IPosition(2,0,0), IPosition(2,nf-1,0)));
    Matrix<double> lastPhase (ap(IPosition(2,0,1), IPosition(2,nf-1,1)));
    Vector<Double> freqs (ampls.asArrayDouble("freqs"));
    Vector<Double> freqw (ampls.asArrayDouble("freqwidths"));
    switch (type) {
    case KEEP:
    case POLAR:
      writeParm (medAmpl, amplValues.name(i), parmdb, freqs, freqw);
      writeParm (lastPhase, name, parmdb, freqs, freqw);
      break;
    case COMPLEX:
      name.gsub (":Phase:", ":Real:");
      writeParm (medAmpl*sin(lastPhase), name, parmdb, freqs, freqw);
      name.gsub (":Real:", ":Imag:");
      writeParm (medAmpl*cos(lastPhase), name, parmdb, freqs, freqw);
      break;
    }
  }
}

void doIt (const String& nameIn, const String& nameOut,
           bool append, int skipLast, bool phase0,
           float amplPerc, parmValType type)
{
  // Open the ParmDBs.
  ParmFacade pdbIn (nameIn);
  ParmDBMeta metaOut ("casa", nameOut);
  ParmDB pdbOut (metaOut, !append);
  vector<double> range = pdbIn.getRange ("Gain:*");
  // Usually real and imaginary are used. Try to read them.
  Record realValues = pdbIn.getValuesGrid ("Gain:*:Real:*",
                                           range[0], range[1],
                                           range[2], range[3]);
  Record imagValues = pdbIn.getValuesGrid ("Gain:*:Imag:*",
                                           range[0], range[1],
                                           range[2], range[3]);
  ASSERT (realValues.size() == imagValues.size());
  if (realValues.size() > 0) {
    processRI (realValues, imagValues, amplPerc, skipLast, phase0, pdbOut, type);
  } else {
    Record amplValues  = pdbIn.getValuesGrid ("Gain:*:Ampl:*",
                                              range[0], range[1],
                                              range[2], range[3]);
    Record phaseValues = pdbIn.getValuesGrid ("Gain:*:Phase:*",
                                              range[0], range[1],
                                              range[2], range[3]);
    ASSERT (amplValues.size() == phaseValues.size());
    ASSERTSTR  (amplValues.size() > 0, "real/imag nor amplitude/phase "
                "parameters found in " << nameIn);
    processAP (amplValues, phaseValues, amplPerc, skipLast, phase0, pdbOut, type);
  }
}

int main (int argc, char *argv[])
{
  // Show version and initialize logger.
  TEST_SHOW_VERSION (argc, argv, ParmDB);
  INIT_LOGGER(basename(string(argv[0])));
  try {
    // Get input arguments.
    InputParSet input;
    input.setVersion ("11-Nov-2011 GvD");
    input.create ("in",  "", "Name of input ParmDB",  "string");
    input.create ("out", "", "Name of output ParmDB", "string");
    input.create ("append", "False", "Append to the output ParmDB?", "bool");
    input.create ("type", "keep", "How to write output: "
                  "keep=same as input, polar=ampl/phase, complex=real/imag",
                  "string");
    input.create ("skiplast", "True", "Ignore last time solution?", "bool");
    input.create ("zerophase", "False", "Make the phase zero?", "bool");
    input.create ("amplperc", "10.",
                   "Print warning if amplitude deviates more than this "
                   "percentage from the median amplitude",
                   "float");
    input.readArguments (argc, argv);
    String nameIn   = input.getString ("in");
    String nameOut  = input.getString ("out");
    bool   append   = input.getBool   ("append");
    bool   phase0   = input.getBool   ("zerophase");
    int    skipLast = (input.getBool  ("skiplast")  ?  1 : 0);
    Double amplPerc = input.getDouble ("amplperc");
    String type     = input.getString ("type");
    parmValType parmType = KEEP;
    type.downcase();
    if (type == "polar") {
      parmType = POLAR;
    } else if (type == "complex") {
      parmType = COMPLEX;
    } else if (type != "keep") {
      throw AipsError("parameter type must have value KEEP, POLAR, or COMPLEX");
    }
    // Do the export.
    doIt (nameIn, nameOut, append, skipLast, phase0, amplPerc, parmType);
  } catch (LOFAR::Exception& ex) {
    cerr << "Caught LOFAR exception: " << ex << endl;
    return 1;
  } catch (casa::AipsError& ex) {
    cerr << "Caught AIPS error: " << ex.what() << endl;
    return 1;
  }
  
  return 0;
}
