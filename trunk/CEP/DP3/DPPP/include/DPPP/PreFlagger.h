//# PreFlagger.h: DPPP step class to average in time and/or freq
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

#ifndef DPPP_PREFLAGGER_H
#define DPPP_PREFLAGGER_H

// @file
// @brief DPPP step class to average in time and/or freq

#include <DPPP/DPInput.h>
#include <DPPP/DPBuffer.h>
#include <Common/lofar_vector.h>

namespace LOFAR {
  class ParameterSet;
  class ParameterValue;

  namespace DPPP {

    // @ingroup NDPPP

    // This class is a DPStep class flagging data points based on data
    // selections given in the parset file.
    // The following selections can be given:
    // <ul>
    //  <li> minimum and/or maximum UV distance
    //  <li> autocorrelations
    //  <li> baselines using names for antenna 1 and 2
    //  <li> antennae using antenna names
    //  <li> channel numbers
    //  <li> frequency ranges
    // </ul>
    // The antenna names can contain shell-style wildcards (* ? [] {}).

    class PreFlagger: public DPStep
    {
    public:
      // Construct the object.
      // Parameters are obtained from the parset using the given prefix.
      // The antenna names are used to find antenna numbers.
      // The channel frequencies as they are in the input step must be given
      // starting at the start-channel.
      PreFlagger (DPInput*, const ParameterSet&, const string& prefix,
                  const casa::Vector<casa::String>& antNames,
                  const casa::Vector<double>& inputChanFreqs);

      virtual ~PreFlagger();

      // Process the data.
      // When processed, it invokes the process function of the next step.
      virtual bool process (const DPBuffer&);

      // Finish the processing of this step and subsequent steps.
      virtual void finish();

      // Update the average info.
      // It is used to adjust the parms if needed.
      virtual void updateAverageInfo (AverageInfo&);

      // Show the step parameters.
      virtual void show (std::ostream&) const;

    private:
      // Set the flags for baselines with mismatching UV distances.
      void flagUV (const casa::Matrix<double>& uvw,
                   casa::Cube<bool>& flags);

      // Set the flags for matching baselines.
      void flagBL (const casa::Vector<int>& ant1,
                   const casa::Vector<int>& ant2,
                   casa::Cube<bool>& flags);

      // Flag the channels given in itsChannels.
      void flagChannels (casa::Cube<bool>& flags);

      // Fill the baseline matrix; set true for baselines to flag.
      void fillBLMatrix (const casa::Vector<casa::String>& antNames);

      // Return a vector with a value per correlation.
      // If no parm value given use the default.
      // If the parm value is a vector, use the given values. Use the
      // default for non-given correlations.
      // If the parm value is a single value, use it for all correlations.
      // <br>If an amplitude is given, itsFlagOnAmpl is set.
      vector<float> fillAmpl (const ParameterValue& value, float defVal);

      // Return a vector with UV ranges.
      // If an UV value is given, itsFlagOnUV is set.
      // It looks for the named parameter suffixed with 'range', 'min', and
      // 'max'. The returned vector contains 2 subsequent values for each range
      // (min and max are also turned into a range).
      // Optionally the values are squared to avoid having to take a sqrt
      // of the data's UV coordinates.
      // <br>If a UV value is given, itsFlagOnUV is set.
      vector<double> fillUV (const ParameterSet& parset,
                             const string& prefix,
                             const string& name,
                             bool square);

      // Update itsFreqs by averaging them as needed.
      void averageFreqs (uint startChan, uint inchanAvg);

      // Handle the frequency ranges given and determine which channels
      // have to be flagged.
      void handleFreqRanges();

      // Get the value and possible unit.
      // If no unit is given, the argument is left untouched.
      void getValue (const string& str, double& value, casa::String& unit);

      // Get the frequency in Hz using the value and unit.
      double getFreqHz (double value, const casa::String& unit);

      //# Data members.
      DPInput*             itsInput;
      string               itsName;
      casa::Vector<double> itsFreqs;    //# frequencies of the input (MS)
      bool                 itsFlagOnUV; //# true = do uv distance based flagging
      bool                 itsFlagOnBL; //# true = do ant/bl based flagging
      bool                 itsFlagOnAmpl; //# true = do amplitude based flagging
      bool                 itsAutoCorr; //# flag autocorrelations?
      casa::Matrix<bool>   itsFlagBL;   //# true = flag baseline [i,j]
      vector<double>       itsRangeUVm; //# strings of UV (in m) to be flagged
      vector<double>       itsRangeUm;  //# strings of U (in m) to be flagged
      vector<double>       itsRangeVm;  //# strings of V (in m) to be flagged
      vector<double>       itsRangeUVl; //# strings of UV (in wl) to be flagged
      vector<double>       itsRangeUl;  //# strings of U (in wl) to be flagged
      vector<double>       itsRangeVl;  //# strings of V (in wl) to be flagged
      vector<float>        itsAmplMin;  //# minimum amplitude for each corr
      vector<float>        itsAmplMax;  //# maximum amplitude for each corr
      vector<uint>         itsChannels; //# channels to be flagged.
      vector<uint>         itsFlagChan; //# channels given to be flagged.
      vector<string>       itsFlagFreq; //# frequency ranges to be flagged
      vector<string>       itsFlagAnt1; //# ant1 patterns of baseline flagging
      vector<string>       itsFlagAnt2; //# ant2 patterns of baseline flagging
      vector<string>       itsFlagAnt;  //# antennae patterns to flag
    };

  } //# end namespace
}

#endif
