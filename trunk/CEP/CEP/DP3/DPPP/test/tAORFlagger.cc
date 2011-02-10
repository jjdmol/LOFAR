//# tAORFlagger.cc: Test program for class AORFlagger
//# Copyright (C) 2010
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
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <DPPP/AORFlagger.h>
#include <DPPP/DPInput.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <DPPP/ParSet.h>
#include <Common/StringUtil.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::DPPP;
using namespace casa;
using namespace std;

// Simple class to generate input arrays.
// It can only set all flags to true or all to false.
// Weights are always 1.
// It can be used with different nr of times, channels, etc.
class TestInput: public DPInput
{
public:
  TestInput(int ntime, int nant, int nchan, int ncorr, bool flag)
    : itsCount(0), itsNTime(ntime), itsNBl(nant*(nant+1)/2), itsNChan(nchan),
      itsNCorr(ncorr), itsFlag(flag)
  {
    // Fill the baseline stations; use 4 stations.
    // So they are called 00 01 02 03 10 11 12 13 20, etc.
    itsAnt1.resize (itsNBl);
    itsAnt2.resize (itsNBl);
    int st1 = 0;
    int st2 = 0;
    for (int i=0; i<itsNBl; ++i) {
      itsAnt1[i] = st1;
      itsAnt2[i] = st2;
      if (++st2 == 4) {
        st2 = 0;
        if (++st1 == 4) {
          st1 = 0;
        }
      }
    }
    itsAntNames.resize(4);
    itsAntNames[0] = "rs01.s01";
    itsAntNames[1] = "rs02.s01";
    itsAntNames[2] = "cs01.s01";
    itsAntNames[3] = "cs01.s02";
    // Define their positions (more or less WSRT RT0-3).
    itsAntPos.resize (4);
    Vector<double> vals(3);
    vals[0] = 3828763; vals[1] = 442449; vals[2] = 5064923;
    itsAntPos[0] = MPosition(Quantum<Vector<double> >(vals,"m"),
                             MPosition::ITRF);
    vals[0] = 3828746; vals[1] = 442592; vals[2] = 5064924;
    itsAntPos[1] = MPosition(Quantum<Vector<double> >(vals,"m"),
                             MPosition::ITRF);
    vals[0] = 3828729; vals[1] = 442735; vals[2] = 5064925;
    itsAntPos[2] = MPosition(Quantum<Vector<double> >(vals,"m"),
                             MPosition::ITRF);
    vals[0] = 3828713; vals[1] = 442878; vals[2] = 5064926;
    itsAntPos[3] = MPosition(Quantum<Vector<double> >(vals,"m"),
                             MPosition::ITRF);
    // Define the frequencies.
    itsChanFreqs.resize (nchan);
    indgen (itsChanFreqs, 1050000., 100000.);
  }
private:
  virtual bool process (const DPBuffer&)
  {
    // Stop when all times are done.
    if (itsCount == itsNTime) {
      return false;
    }
    cout << "Input step " << itsCount << ' '<< itsCount*5+2<<endl;
    Cube<Complex> data(itsNCorr, itsNChan, itsNBl);
    for (int i=0; i<int(data.size()); ++i) {
      data.data()[i] = Complex(1.6, 0.9);
    }
    if (itsCount == 5) {
      data += Complex(10.,10.);
    }
    DPBuffer buf;
    buf.setTime (itsCount*5 + 2);   //same interval as in updateAveragInfo
    buf.setData (data);
    Cube<float> weights(data.shape());
    weights = 1.;
    buf.setWeights (weights);
    Cube<bool> flags(data.shape());
    flags = itsFlag;
    buf.setFlags (flags);
    // The fullRes flags are a copy of the XX flags, but differently shaped.
    // They are not averaged, thus only 1 time per row.
    Cube<bool> fullResFlags(itsNChan, 1, itsNBl);
    fullResFlags = itsFlag;
    buf.setFullResFlags (fullResFlags);
    getNextStep()->process (buf);
    ++itsCount;
    return true;
  }

  virtual void finish() {getNextStep()->finish();}
  virtual void show (std::ostream&) const {}
  virtual void updateInfo (DPInfo& info)
    // Use startchan=0 and timeInterval=5
    { info.init (itsNCorr, 0, itsNChan, itsNBl, itsNTime, 5); }

  int itsCount, itsNTime, itsNBl, itsNChan, itsNCorr;
  bool itsFlag;
};

// Class to check result.
class TestOutput: public DPStep
{
public:
  TestOutput(int ntime, int nant, int nchan, int ncorr,
             bool flag, bool useAutoCorr, bool shortbl)
    : itsCount(0), itsNTime(ntime), itsNBl(nant*(nant+1)/2), itsNChan(nchan),
      itsNCorr(ncorr),
      itsFlag(flag),
      itsUseAutoCorr(useAutoCorr),
      itsShortBL(shortbl)
  {}
private:
  virtual bool process (const DPBuffer& buf)
  {
    cout << "Output step " << itsCount << ' '<<itsCount*5+2<<endl;
    // Fill expected result in similar way as TestInput.
    Cube<Complex> result(itsNCorr,itsNChan,itsNBl);
    for (int i=0; i<int(result.size()); ++i) {
      result.data()[i] = Complex(1.6, 0.9);
    }
    if (itsCount == 5) {
      result += Complex(10.,10.);
    }
    // Check the result.
    ///cout << buf.getData()<< result;
    ASSERT (allNear(real(buf.getData()), real(result), 1e-10));
    ASSERT (allNear(imag(buf.getData()), imag(result), 1e-10));
    ASSERT (near(buf.getTime(), 2+5.*itsCount));
    ++itsCount;
    return true;
  }

  virtual void finish() {}
  virtual void show (std::ostream&) const {}
  virtual void updateInfo (DPInfo& info)
  {
    ASSERT (info.startChan()==0);
    ASSERT (int(info.origNChan())==itsNChan);
    ASSERT (int(info.nchan())==itsNChan);
    ASSERT (int(info.ntime())==itsNTime);
    ASSERT (info.timeInterval()==5);
    ASSERT (int(info.nchanAvg())==1);
    ASSERT (int(info.ntimeAvg())==1);
  }

  int itsCount;
  int itsNTime, itsNBl, itsNChan, itsNCorr, itsNAvgTime, itsNAvgChan;
  bool itsFlag, itsUseAutoCorr, itsShortBL;
};


// Execute steps.
void execute (const DPStep::ShPtr& step1)
{
  // Set DPInfo.
  DPInfo info;
  DPStep::ShPtr step = step1;
  while (step) {
    step->updateInfo (info);
    step = step->getNextStep();
  }
  // Execute the steps.
  DPBuffer buf;
  while (step1->process(buf));
  step1->finish();
  step = step1;
  while (step) {
    step->showCounts (cout);
    step = step->getNextStep();
  }
}

// Test simple flagging with or without preflagged points.
void test1(int ntime, int nant, int nchan, int ncorr, bool flag, int threshold,
           bool shortbl)
{
  cout << "test1: ntime=" << ntime << " nrant=" << nant << " nchan=" << nchan
       << " ncorr=" << ncorr << " threshold=" << threshold
       << " shortbl=" << shortbl << endl;
  // Create the steps.
  TestInput* in = new TestInput(ntime, nant, nchan, ncorr, flag);
  DPStep::ShPtr step1(in);
  ParameterSet parset;
  parset.add ("freqwindow", "1");
  parset.add ("timewindow", "1");
  parset.add ("threshold", toString(threshold));
  if (shortbl) {
    parset.add ("blmin", "0");
    parset.add ("blmax", "145");
  }
  DPStep::ShPtr step2(new AORFlagger(in, parset, ""));
  DPStep::ShPtr step3(new TestOutput(ntime, nant, nchan, ncorr, flag, false,
                                     shortbl));
  step1->setNextStep (step2);
  step2->setNextStep (step3);
  step2->show (cout);
  execute (step1);
}

// Test applyautocorr flagging with or without preflagged points.
void test2(int ntime, int nant, int nchan, int ncorr, bool flag, int threshold,
           bool shortbl)
{
  cout << "test2: ntime=" << ntime << " nrant=" << nant << " nchan=" << nchan
       << " ncorr=" << ncorr << " threshold=" << threshold
       << " shortbl=" << shortbl << endl;
  // Create the steps.
  TestInput* in = new TestInput(ntime, nant, nchan, ncorr, flag);
  DPStep::ShPtr step1(in);
  ParameterSet parset;
  parset.add ("freqwindow", "4");
  parset.add ("timewindow", "100");
  parset.add ("padding", "10");
  parset.add ("threshold", toString(threshold));
  parset.add ("applyautocorr", "True");
  if (shortbl) {
    parset.add ("blmax", "145");
  }
  DPStep::ShPtr step2(new AORFlagger(in, parset, ""));
  DPStep::ShPtr step3(new TestOutput(ntime, nant, nchan, ncorr, flag, true,
                                     shortbl));
  step1->setNextStep (step2);
  step2->setNextStep (step3);
  execute (step1);
}


int main()
{
  INIT_LOGGER ("tAORFlagger");
  try {

    for (uint i=0; i<2; ++i) {
      test1(10, 2, 32, 4, false, 1, i>0);
      test1(10, 5, 32, 4, true, 1, i>0);
      test2( 4, 2,  8, 4, false, 100, i>0);
      test2(10, 5, 32, 4, true, 1, i>0);
      test2( 8, 2,  8, 4, false, 100, i>0);
      test2(14, 2,  8, 4, false, 100, i>0);
      ///      test2(99, 8, 64, 4, false, 100, i>0);
    }
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
