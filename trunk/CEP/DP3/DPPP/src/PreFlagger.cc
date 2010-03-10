//# PreFlagger.cc: DPPP step class to flag data on channel, baseline, time
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
#include <DPPP/PreFlagger.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/AverageInfo.h>
#include <Common/ParameterSet.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/GenSort.h>
#include <iostream>
#include <algorithm>
#include <functional>

using namespace casa;

namespace LOFAR {
  namespace DPPP {

    PreFlagger::PreFlagger (DPInput* input,
                            const ParameterSet& parset, const string& prefix)
      : itsName (prefix),
        itsPSet (input, parset, prefix)
    {}

    PreFlagger::~PreFlagger()
    {}

    void PreFlagger::show (std::ostream& os) const
    {
      itsPSet.show (os);
    }

    void PreFlagger::showTimings (std::ostream& os, double duration) const
    {
      os << "  ";
      FlagCounter::showPerc1 (os, itsTimer.getElapsed(), duration);
      os << " MSWriter " << itsName << endl;
    }

    void PreFlagger::updateAverageInfo (AverageInfo& info)
    {
      itsPSet.updateInfo (info);
    }

    bool PreFlagger::process (const DPBuffer& buf)
    {
      itsTimer.start();
      DPBuffer out(buf);
      // The flags will be changed, so make sure we have a unique array.
      out.getFlags().unique();
      // Do the PSet steps and OR the resul with the current flags.
      const Cube<bool>& flags = itsPSet.process (out, Block<bool>());
      transformInPlace (out.getFlags().cbegin(), out.getFlags().cend(),
                        flags.cbegin(), std::logical_or<bool>());
      itsTimer.stop();
      // Let the next step do its processing.
      getNextStep()->process (out);
      return true;
    }

    void PreFlagger::finish()
    {
      // Let the next step finish its processing.
      getNextStep()->finish();
    }


    PreFlagger::PSet::PSet (DPInput* input,
                            const ParameterSet& parset, const string& prefix)
      : itsInput (input),
        itsName  (prefix),
        itsFlagOnUV    (false),
        itsFlagOnBL    (false),
        itsFlagOnAmpl  (false),
        itsFlagOnPhase (false),
        itsFlagOnRI    (false),
        itsFlagOnAzEl  (false),
        itsFlagOnLST   (false)
    {
      itsCorrType = parset.getString       (prefix+"corrtype", string());
      itsStrBL    = parset.getString       (prefix+"baseline", string());
      itsMinUV    = parset.getDouble       (prefix+"uvmmin", -1);
      itsMaxUV    = parset.getDouble       (prefix+"uvmmax", -1);
      itsFlagFreq = parset.getStringVector (prefix+"freqrange",
                                            vector<string>());
      itsFlagChan = parset.getUintVector   (prefix+"chan", vector<uint>(),
                                            true);   // expand .. etc.
      itsAmplMin = fillValuePerCorr
        (ParameterValue (parset.getString  (prefix+"amplmin", string())), 0.,
         itsFlagOnAmpl);
      itsAmplMax  = fillValuePerCorr
        (ParameterValue (parset.getString  (prefix+"amplmax", string())), 1e30,
         itsFlagOnAmpl);
      itsPhaseMin = fillValuePerCorr
        (ParameterValue (parset.getString  (prefix+"phasemin", string())), 0.,
         itsFlagOnPhase);
      itsPhaseMax  = fillValuePerCorr
        (ParameterValue (parset.getString  (prefix+"phasemax", string())), 1e30,
         itsFlagOnPhase);
      itsRealMin = fillValuePerCorr
        (ParameterValue (parset.getString  (prefix+"realmin", string())), 0.,
         itsFlagOnRI);
      itsRealMax  = fillValuePerCorr
        (ParameterValue (parset.getString  (prefix+"realmax", string())), 1e30,
         itsFlagOnRI);
      itsImagMin = fillValuePerCorr
        (ParameterValue (parset.getString  (prefix+"imagmin", string())), 0.,
         itsFlagOnRI);
      itsImagMax  = fillValuePerCorr
        (ParameterValue (parset.getString  (prefix+"imagmax", string())), 1e30,
         itsFlagOnRI);
      // Fill the matrix with the baselines to flag.
      fillBLMatrix (itsInput->antennaNames());
      // Get the possible times and other info to flag on.
      readTimeParms (parset);
      // Determine if to flag on UV distance.
      // If so, square the distances to avoid having to take the sqrt in flagUV.
      itsFlagOnUV = itsMinUV > 0;
      itsMinUV   *= itsMinUV;
      if (itsMaxUV > 0) {
        itsFlagOnUV = true;
        itsMaxUV   *= itsMaxUV;
      } else {
        // Make it a very high number.
        itsMaxUV = 1e30;
      }
      // Read the possible child steps.
      vector<string> psets = parset.getStringVector (prefix+"sets",
                                                     vector<string>());
      for (uint i=0; i<psets.size(); ++i) {
        itsPSets.push_back
          (PSet::ShPtr(new PSet(itsInput, parset, prefix+psets[i]+'.')));
      }
    }

    void PreFlagger::PSet::readTimeParms (const ParameterSet& parset)
    {
    }

    void PreFlagger::PSet::updateInfo (const AverageInfo& info)
    {
      uint nrcorr = info.ncorr();
      uint nrchan = info.nchan();
      itsFlags.resize (nrcorr, nrchan, info.nbaselines());
      itsMatchBL.resize (info.nbaselines());
      // Check for channels exceeding nr of channels.
      itsChannels.reserve (itsFlagChan.size());
      for (uint i=0; i<itsFlagChan.size(); ++i) {
        if (itsFlagChan[i] < info.nchan()) {
          itsChannels.push_back (itsFlagChan[i]);
        }
      }
      // Now handle the possibly given frequency ranges and add the
      // resulting channels to itsChannels.
      if (! itsFlagFreq.empty()) {
        // Convert the given frequencies to possibly averaged frequencies.
        handleFreqRanges (itsInput->chanFreqs (info.nchanAvg()));
      }
      // Sort uniquely and resize as needed.
      uint nr = GenSort<uint>::sort (&(itsChannels[0]), itsChannels.size(),
                                     Sort::Ascending,
                                     Sort::QuickSort + Sort::NoDuplicates);
      itsChannels.resize (nr);
      // Turn the channels into a mask.
      itsChanFlags.resize (nrcorr, nrchan);
      itsChanFlags = false;
      for (uint i=0; i<nr; ++i) {
        uint chan = itsChannels[i];
        for (uint j=0; j<nrcorr; ++j) {
          itsChanFlags(j,chan) = true;
        }
      }
      // Do it for the child steps.
      for (uint i=0; i<itsPSets.size(); ++i) {
        itsPSets[i]->updateInfo (info);
      }
    }

    void PreFlagger::PSet::show (std::ostream& os) const
    {
      os << "PreFlagger set " << itsName << std::endl;
      os << "  baseline:       " << itsStrBL << std::endl;
      os << "  corrtype:       " << itsCorrType << std::endl;
      os << "  uvmmin:         " << sqrt(itsMinUV) << std::endl;
      os << "  uvmmax:         " << sqrt(itsMaxUV) << std::endl;
      os << "  chan:           " << itsFlagChan << std::endl;
      os << "  freqrange:      " << itsFlagFreq << std::endl;
      if (! itsChannels.empty()) {
        os << "   chan to flag:  " << itsChannels << std::endl;
      }
      // Do it for the child steps.
      for (uint i=0; i<itsPSets.size(); ++i) {
        itsPSets[i]->show (os);
      }
    }

    const Cube<bool>& PreFlagger::PSet::process (DPBuffer& out,
                                                 const Block<bool>& matchBL)
    {
      const IPosition& shape = out.getFlags().shape();
      uint nr = shape[0] * shape[1];
      // Take over the baseline info from the parent. Default is all.
      if (matchBL.empty()) {
        itsMatchBL = true;
      } else{
        itsMatchBL = matchBL;
      }
      // The PSet tree is a combination of ORs and ANDs.
      // Depth is AND, breadth is OR.
      // In each pset flagging is done in two stages.
      // First it is determined which baselines are not flagged. It is kept
      // in the itsMatchBL block.
      // This is passed to the children who do their flagging. In this way
      // a child can minimize the amount of work to do.
      // Each child passes back its flags; the flags are or-ed.

      // First flag on baseline if necessary.
      if (itsFlagOnBL) {
        flagBL (itsInput->getAnt1(), itsInput->getAnt2());
      }
      // Flag on UV distance if necessary.
      if (itsFlagOnUV) {
        flagUV (itsInput->fetchUVW(out, out.getRowNrs()));
      }
      // Flag on AzEl is necessary.
      if (itsFlagOnAzEl) {
        flagAzEl ();
      }
      // Convert each baseline flag to a flag per correlation/channel.
      bool* flagPtr = itsFlags.data();
      for (uint i=0; i<itsMatchBL.size(); ++i) {
        std::fill (flagPtr, flagPtr+nr, itsMatchBL[i]);
        flagPtr += nr;
      }
      // Flag on channel if necessary.
      if (! itsChannels.empty()) {
        flagChannels();
      }
      // Flag on amplitude, phase or real/imaginary if necessary.
      if (itsFlagOnAmpl) {
        if (out.getAmplitudes().empty()) {
          out.setAmplitudes (amplitude(out.getData()));
        }
        flagAmpl (out.getAmplitudes());
      }
      if (itsFlagOnRI) {
        flagComplex (out.getData());
      }
      if (itsFlagOnPhase) {
        flagPhase (out.getData());
      }
      // Let the lower psets do their flagging and combine their flags.
      // Use the result of the first one as the buffer to OR in.
      // This buffer can be used without any problem, because the child
      // will reinitialize it when used again.
      if (! itsPSets.empty()) {
        Cube<bool> mflags (itsPSets[0]->process (out, itsMatchBL));
        for (uint i=1; i<itsPSets.size(); ++i) {
          const Cube<bool>& flags = itsPSets[i]->process (out, itsMatchBL);
          // No ||= operator exists, so use the transform function.
          transformInPlace (mflags.cbegin(), mflags.cend(),
                            flags.cbegin(), std::logical_or<bool>());
        }
        // Finally AND the children's flags with the flags of this pset.
        transformInPlace (itsFlags.cbegin(), itsFlags.cend(),
                          mflags.cbegin(), std::logical_and<bool>());
      }
      return itsFlags;
    }

    void PreFlagger::PSet::flagUV (const Matrix<double>& uvw)
    {
      uint nrbl = itsMatchBL.size();
      const double* uvwPtr = uvw.data();
      for (uint i=0; i<nrbl; ++i) {
        if (itsMatchBL[i]) {
          // UV-distance is sqrt(u^2 + v^2).
          // The sqrt is not needed because minuv and maxuv are squared.
          double uvdist = uvwPtr[0] * uvwPtr[0] + uvwPtr[1] * uvwPtr[1];
          if (uvdist >= itsMinUV  ||  uvdist <= itsMaxUV) {
            // UV-dist mismatches, so do not flag baseline.
            itsMatchBL[i] = false;
          }
        }
        uvwPtr += 3;
      }
    }

    void PreFlagger::PSet::flagBL (const Vector<int>& ant1,
                                   const Vector<int>& ant2)
    {
      uint nrbl = itsMatchBL.size();
      const int* ant1Ptr = ant1.data();
      const int* ant2Ptr = ant2.data();
      for (uint i=0; i<nrbl; ++i) {
        if (itsMatchBL[i]) {
          if (! itsFlagBL(*ant1Ptr, *ant2Ptr)) {
            // do not flag this baseline
            itsMatchBL[i] = false;
          }
        }
        ant1Ptr++;
        ant2Ptr++;
      }
    }

    void PreFlagger::PSet::flagAzEl ()
    {
    }

    void PreFlagger::PSet::flagAmpl (const Cube<float>& values)
    {
      const IPosition& shape = values.shape();
      uint nrcorr = shape[0];
      uint nr = shape[1] * shape[2];
      const float* valPtr = values.data();
      bool* flagPtr = itsFlags.data();
      for (uint i=0; i<nr; ++i) {
        bool flag = false;
        for (uint j=0; j<nrcorr; ++j) {
          if (*valPtr < itsAmplMin[j]  ||  *valPtr > itsAmplMax[j]) {
            flag = true;
            break;
          }
        }
        if (!flag) {
          for (uint j=0; j<nrcorr; ++j) {
            flagPtr[j] = false;
          }
        }
        valPtr  += nrcorr;
        flagPtr += nrcorr;
      }
    }

    void PreFlagger::PSet::flagPhase (const Cube<Complex>& values)
    {
      const IPosition& shape = values.shape();
      uint nrcorr = shape[0];
      uint nr = shape[1] * shape[2];
      const Complex* valPtr = values.data();
      bool* flagPtr = itsFlags.data();
      for (uint i=0; i<nr; ++i) {
        bool flag = false;
        for (uint j=0; j<nrcorr; ++j) {
          float phase = arg(valPtr[j]);
          if (phase < itsPhaseMin[j]  ||  phase > itsPhaseMax[j]) {
            flag = true;
            break;
          }
        }
        if (!flag) {
          for (uint j=0; j<nrcorr; ++j) {
            flagPtr[j] = false;
          }
        }
        valPtr  += nrcorr;
        flagPtr += nrcorr;
      }
    }

    void PreFlagger::PSet::flagComplex (const Cube<Complex>& values)
    {
      const IPosition& shape = values.shape();
      uint nrcorr = shape[0];
      uint nr = shape[1] * shape[2];
      const Complex* valPtr = values.data();
      bool* flagPtr = itsFlags.data();
      for (uint i=0; i<nr; ++i) {
        bool flag = false;
        for (uint j=0; j<nrcorr; ++j) {
          if (valPtr[j].real() < itsRealMin[j]  ||
              valPtr[j].real() > itsRealMax[j]  ||
              valPtr[j].imag() < itsImagMin[j]  ||
              valPtr[j].imag() > itsImagMax[j]) {
            flag = true;
            break;
          }
        }
        if (!flag) {
          for (uint j=0; j<nrcorr; ++j) {
            flagPtr[j] = false;
          }
        }
        valPtr  += nrcorr;
        flagPtr += nrcorr;
      }
    }

    void PreFlagger::PSet::flagChannels()
    {
      const IPosition& shape = itsFlags.shape();
      uint nr   = shape[0] * shape[1];
      uint nrbl = shape[2];
      bool* flagPtr = itsFlags.data();
      for (uint i=0; i<nrbl; ++i) {
        transformInPlace (flagPtr, flagPtr+nr,
                          itsChanFlags.cbegin(), std::logical_and<bool>());
        flagPtr += nr;
      }
    }

    vector<float> PreFlagger::PSet::fillValuePerCorr
    (const ParameterValue& value, float defVal, bool& doFlag)
    {
      // Initialize with the default value per correlation.
      vector<float> result(4);
      std::fill (result.begin(), result.end(), defVal);
      if (! value.get().empty()) {
        // It contains a value, so set that flagging is done.
        doFlag = true;
        if (value.isVector()) {
          // Defined as a vector, take the values given.
          vector<float> vals = value.getFloatVector();
          uint sz = std::min(vals.size(), result.size());
          for (uint i=0; i<sz; ++i) {
            result[i] = vals[i];
          }
        } else {
          // A single value means use it for all correlations.
          std::fill (result.begin(), result.end(), value.getFloat());
        }
      }
      return result;
    }

    void PreFlagger::PSet::fillBLMatrix (const Vector<String>& antNames)
    {
      // Initialize the matrix.
      itsFlagBL.resize (antNames.size(), antNames.size());
      itsFlagBL = false;
      // Set to true if autocorrelations.
      string corrType = toLower(itsCorrType);
      if (corrType == "auto") {
        itsFlagOnBL = true;
        itsFlagBL.diagonal() = true;
      } else if (corrType == "cross") {
        itsFlagOnBL = true;
        itsFlagBL   = true;
        itsFlagBL.diagonal() = false;
      } else {
        ASSERTSTR (corrType.empty(), "PreFlagger corrType " << itsCorrType
                   << " is invalid; must be auto, cross or ''");
      }
      // Loop through all values in the baseline string.
      if (! itsStrBL.empty()) {
        itsFlagOnBL = true;
        vector<ParameterValue> pairs = ParameterValue(itsStrBL).getVector();
        // Each ParameterValue can be a single value (antenna) or a pair of
        // values (a baseline).
        // Note that [ant1,ant2] is somewhat ambiguous; it means two antennae,
        // but one might think it means a baseline [[ant1,ant2]].
        if (pairs.size() == 2  &&
            !(pairs[0].isVector()  ||  pairs[1].isVector())) {
          LOG_WARN_STR ("PreFlagger baseline " << itsStrBL
                        << " means two antennae, but is somewhat ambigious; "
                        << "it's more clear to use [[ant1],[ant2]]");
        }
        for (uint i=0; i<pairs.size(); ++i) {
          vector<string> bl = pairs[i].getStringVector();
          if (bl.size() == 1) {
            // Turn the given antenna name pattern into a regex.
            Regex regex(Regex::fromPattern (bl[0]));
            // Loop through all antenna names and set matrix for matching ones.
            for (uint i2=0; i2<antNames.size(); ++i2) {
              if (antNames[i2].matches (regex)) {
                // Antenna matches, so set all corresponding flags.
                for (uint j=0; j<antNames.size(); ++j) {
                  itsFlagBL(i2,j) = true;
                  itsFlagBL(j,i2) = true;
                }
              }
            }
          } else {
            ASSERTSTR (bl.size() == 2, "PreFlagger baseline " << bl <<
                       " should contain 1 or 2 antenna name patterns");
            // Turn the given antenna name pattern into a regex.
            Regex regex1(Regex::fromPattern (bl[0]));
            Regex regex2(Regex::fromPattern (bl[1]));
            // Loop through all antenna names and set matrix for matching ones.
            for (uint i2=0; i2<antNames.size(); ++i2) {
              if (antNames[i2].matches (regex2)) {
                // Antenna2 matches, now try Antenna1.
                for (uint i1=0; i1<antNames.size(); ++i1) {
                  if (antNames[i1].matches (regex1)) {
                    itsFlagBL(i1,i2) = true;
                  }
                }
              }
            }
          }
        }
      }
    }

    void PreFlagger::PSet::handleFreqRanges (const Vector<double>& chanFreqs)
    {
      // A frequency range can be given as  value..value or value+-value.
      // Units can be given for each value; if one is given it applies to both.
      // Default unit is MHz.
      for (vector<string>::const_iterator str = itsFlagFreq.begin();
           str != itsFlagFreq.end(); ++str) {
        // Find the .. or +- token.
        bool usepm = false;
        string::size_type pos;
        pos = str->find ("..");
        if (pos == string::npos) {
          usepm = true;
          pos = str->find ("+-");
          ASSERTSTR (pos != string::npos, "PreFlagger freqrange '" << *str
                     << "' should be range using .. or +-");
        }
        string str1 = str->substr (0, pos);
        string str2 = str->substr (pos+2);
        String u1, u2;
        double v1, v2;
        getValue (str1, v1, u1);
        // Default unit for 2nd value is that of 1st value.
        u2 = u1;
        getValue (str2, v2, u2);
        // If no unit, use MHz.
        if (u2.empty()) {
          u2 = "MHz";
        }
        // Default unit of 1st value is that of 2nd value.
        if (u1.empty()) {
          u1 = u2;
        }
        v1 = getFreqHz (v1, u1);
        v2 = getFreqHz (v2, u2);
        if (usepm) {
          double pm = v2;
          v2 = v1 + pm;
          v1 -= pm;
        }
        // Add any channel inside this range.
        for (uint i=0; i<chanFreqs.size(); ++i) {
          if (chanFreqs[i] > v1  &&  chanFreqs[i] < v2) {
            itsChannels.push_back (i);
          }
        }
      }
    }

    void PreFlagger::PSet::getValue (const string& str, double& value,
                                     String& unit)
    {
      // See if a unit is given at the end.
      String v(str);
      // Remove possible trailing blanks.
      rtrim(v);
      Regex regex("[a-zA-Z]+$");
      string::size_type pos = v.index (regex);
      if (pos != String::npos) {
        unit = v.from   (pos);
        v    = v.before (pos);
      }
      // Set value and unit.
      value = strToDouble(v);
    }

    double PreFlagger::PSet::getFreqHz (double value, const String& unit)
    {
      Quantity q(value, unit);
      return q.getValue ("Hz");
    }

  } //# end namespace
}
