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

#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/MUString.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>
#include <pwd.h>
#include <unistd.h>
#include <libgen.h>

using namespace casa;
using namespace LOFAR;
using namespace BBS;

// Create a new parm for export.
void writeAsValue (const String& parmName, const Matrix<double>& values,
                   ParmDB& parmdb,
                   const Vector<double>& freqs, const Vector<double>& freqw)
{
  // There should be one time slot.
  ASSERT (values.shape()[1] == 1);
  // Form the grid.
  Axis::ShPtr timeAxis(new RegularAxis(-1e30, 1e30, 1, true));
  Axis::ShPtr freqAxis;
  if (allNear (freqw, freqw[0], 1e-5)) {
    freqAxis = Axis::ShPtr(new RegularAxis(freqs[0]+freqw[0]*0.5,
                                           freqw[0], freqs.size()));
  } else {
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
  int nameId;
  parmdb.putValues (parmName, nameId, pvset);
  cout << "Wrote new record for parameter " << parmName << endl;
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
  cout << "Wrote new defaultvalue record for parameter " << parmName << endl;
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
                      double perc, int skipLast)
{
  // Get the median in time for each frequency.
  ASSERT (ampl.shape() == phase.shape());
  const IPosition& shp = ampl.shape();
  int nf = shp[0];
  int nt = shp[1] - skipLast;
  Matrix<double> result(nf,2);
  for (int i=0; i<nf; ++i) {
    // Ignore the last value.
    Matrix<double> aline = ampl(IPosition(2,i,0), IPosition(2,i,nt-1));
    double med = median(aline);
    if (abs(aline(i,nt-1) - med) > perc/100*med) {
      cout << "amplitude " << aline(i,nt-1) << " differs more than "
           << perc << "% from median " << med << endl;
    }
    result(i,0) = med;
    // Use one but last phase value.
    result(i,1) = phase(i, nt-1);
  }
  return result;
}

void processRI (const Record& realValues, const Record& imagValues,
                double perc, int skipLast, ParmDB& parmdb)
{
  Vector<Double> freqs = realValues.asRecord(0).asArrayDouble("freqs");
  Vector<Double> freqw = realValues.asRecord(0).asArrayDouble("freqwidths");
  Vector<Double> times = realValues.asRecord(0).asArrayDouble("times");
  for (uInt i=0; i<realValues.nfields(); ++i) {
    String name = realValues.name(i);
    // Replace real by imaginary to find its values.
    name.gsub ("Real", "Imag");
    const RecordInterface& reals = realValues.asRecord(i);
    const RecordInterface& imags = imagValues.asRecord(name);
    Matrix<double> real (reals.asArrayDouble("values"));
    Matrix<double> imag (imags.asArrayDouble("values"));
    int nf = real.shape()[0];
    // Get the median ampl and last phase per frequency.
    Matrix<double> ap (getAP (sqrt(real*real + imag*imag),
                              atan2(real, imag),
                              perc, skipLast));
    // Write the results as real/imag.
    Matrix<double> medAmpl   (ap(IPosition(2,0,0), IPosition(2,nf-1,0)));
    Matrix<double> lastPhase (ap(IPosition(2,0,1), IPosition(2,nf-1,1)));
    writeParm (medAmpl*sin(lastPhase), realValues.name(i), parmdb,
               freqs, freqw);
    writeParm (medAmpl*cos(lastPhase), name, parmdb, freqs, freqw);
  }
}

void processAP (const Record& amplValues, const Record& phaseValues,
                double perc, int skipLast, ParmDB& parmdb)
{
  Vector<Double> freqs = amplValues.asRecord(0).asArrayDouble("freqs");
  Vector<Double> freqw = amplValues.asRecord(0).asArrayDouble("freqwidths");
  Vector<Double> times = amplValues.asRecord(0).asArrayDouble("times");
  for (uInt i=0; i<amplValues.nfields(); ++i) {
    String name = amplValues.name(i);
    // Replace amplitude by phase to find its values.
    name.gsub ("Ampl", "Phase");
    const RecordInterface& ampls  = amplValues.asRecord(i);
    const RecordInterface& phases = phaseValues.asRecord(name);
    Matrix<double> ampl  (ampls.asArrayDouble("values"));
    Matrix<double> phase (phases.asArrayDouble("values"));
    int nf = ampl.shape()[0];
    // Get the median ampl and last phase per frequency.
    Matrix<double> ap (getAP (ampl, phase, perc, skipLast));
    // Write the results as ampl/phase.
    Matrix<double> medAmpl   (ap(IPosition(2,0,0), IPosition(2,nf-1,0)));
    Matrix<double> lastPhase (ap(IPosition(2,0,1), IPosition(2,nf-1,1)));
    writeParm (medAmpl, amplValues.name(i), parmdb, freqs, freqw);
    writeParm (lastPhase, name, parmdb, freqs, freqw);
  }
}

void doIt (const String& nameIn, const String& nameOut,
           Bool append, int skipLast, float amplPerc)
{
  // Open the ParmDBs.
  ParmFacade pdbIn (nameIn);
  ParmDBMeta metaOut ("casa", nameOut);
  ParmDB pdbOut (metaOut, !append);
  vector<double> range = pdbIn.getRange ("Gain:*");
  Record realValues = pdbIn.getValues ("Gain:*:Real:*",
                                       range[0], range[1],
                                       range[2], range[3]);
  Record imagValues = pdbIn.getValues ("Gain:*:Imag:*",
                                       range[0], range[1],
                                       range[2], range[3]);
  // Usually real and imaginary are used. Try to read them.
  ///  Record realValues = pdbIn.getValuesGrid ("Gain:*:Real*");
  ///  Record imagValues = pdbIn.getValuesGrid ("Gain:*:Imag:*");
  ASSERT (realValues.size() == imagValues.size());
  if (realValues.size() > 0) {
    processRI (realValues, imagValues, amplPerc, skipLast, pdbOut);
  } else {
    Record amplValues  = pdbIn.getValues ("Gain:*:Ampl:*",
                                          range[0], range[1],
                                          range[2], range[3]);
    Record phaseValues = pdbIn.getValues ("Gain:*:Phase:*",
                                          range[0], range[1],
                                          range[2], range[3]);
    ASSERT (amplValues.size() == phaseValues.size());
    ASSERTSTR  (amplValues.size() > 0, "real/imag nor amplitude/phase "
                "parameters found in " << nameIn);
    processAP (amplValues, phaseValues, amplPerc, skipLast, pdbOut);
  }
}

int main (int argc, char *argv[])
{
  // Show version and initialize logger.
  const char* progName = basename(argv[0]);
  TEST_SHOW_VERSION (argc, argv, ParmDB);
  INIT_LOGGER(progName);
  try {
    // Get input arguments.
    InputParSet input;
    input.setVersion ("20-Oct-2011 GvD");
    input.create ("in",  "", "Name of input ParmDB",  "string");
    input.create ("out", "", "Name of output ParmDB", "string");
    input.create ("append", "False", "Append to the output ParmDB", "bool");
    input.create ("skiplast", "True", "Ignore last time solution?", "bool");
    input.create ("amplperc", "10.",
                   "Print warning if amplitude deviates more than this "
                   "percentage from the median amplitude",
                   "float");
    input.readArguments (argc, argv);
    String nameIn   = input.getString ("in");
    String nameOut  = input.getString ("out");
    Bool   append   = input.getBool ("append");
    int    skipLast = (input.getBool("skiplast")  ?  1 : 0);
    Double amplPerc = input.getDouble ("amplperc");
    // Do the export.
    doIt (nameIn, nameOut, append, skipLast, amplPerc);
  } catch (std::exception& x) {
    cerr << "Caught exception: " << x.what() << endl;
    return 1;
  }
  
  return 0;
}
