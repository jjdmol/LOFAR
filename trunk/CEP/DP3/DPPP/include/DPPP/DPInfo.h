//# DPInfo.h: General info about DPPP data processing attributes like averaging
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

#ifndef DPPP_DPINFO_H
#define DPPP_DPINFO_H

// @file
// @brief General info about DPPP data processing attributes like averaging

#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasureHolder.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>

namespace LOFAR {
  namespace DPPP {

    //# Forward declarations.
    class DPInput;

    // @ingroup NDPPP

    // This class contains the information about the number of correlations,
    // channels, baselines, and times.
    // It is initialized by the first step and updated by steps like
    // Averager that change the number of channels or times.
    // Steps can take information from it to know about shapes.

    class DPInfo
    {
    public:

      // Define bits telling if data and/or flags need to be written.
      enum NeedWrite {NeedWriteData=1, NeedWriteFlags=2, NeedWriteWeight=4};

      // Default constructor.
      DPInfo();

      // Set the initial info from the input.
      void init (uint ncorr, uint nchan,
                 uint ntime, double startTime, double timeInterval,
                 const string& msName, const string& antennaSet);

      // Set nr of channels.
      void setNChan (uint nchan)
        { itsNChan = nchan; }

      // Set the frequency info.
      // An empty resolutions or effectiveBW is default to chanWidths.
      // If totalBW is 0, it is set to the sum of effectiveBW.
      // If refFreq is 0, it is set to the middle of chanFreqs (mean if even).
      void set (const casa::Vector<double>& chanFreqs,
                const casa::Vector<double>& chanWidths,
                const casa::Vector<double>& resolutions= casa::Vector<double>(),
                const casa::Vector<double>& effectiveBW= casa::Vector<double>(),
                double totalBW = 0,
                double refFreq = 0);

      // Set array info.
      void set (const casa::MPosition& arrayPos,
                const casa::MDirection& phaseCenter,
                const casa::MDirection& delayCenter,
                const casa::MDirection& tileBeamDir);

      // Set the info for the given antennae and baselines.
      void set (const casa::Vector<casa::String>& antNames,
                const casa::Vector<casa::Double>& antDiam,
                const vector<casa::MPosition>& antPos,
                const casa::Vector<casa::Int>& ant1,
                const casa::Vector<casa::Int>& ant2);

      // Update the info for the given average factors.
      // If chanAvg is higher than the actual nr of channels, it is reset.
      // The same is true for timeAvg.
      // It returns the possibly reset nr of channels to average.
      uint update (uint chanAvg, uint timeAvg);

      // Update the info from the given selection parameters.
      // Optionally unused stations are really removed from the antenna lists.
      void update (uint startChan, uint nchan,
                   const vector<uint>& baselines, bool remove);

      // Remove unused stations from the antenna lists.
      void removeUnusedAnt();

      // Set the phase center.
      // If original=true, it is set to the original phase center.
      void setPhaseCenter (const casa::MDirection& phaseCenter, bool original)
        { itsPhaseCenter=phaseCenter; itsPhaseCenterIsOriginal = original; }


      // Get the info.
      const string& msName() const
        { return itsMSName; }
      const string& antennaSet() const
        { return itsAntennaSet; }
      uint ncorr() const
        { return itsNCorr; }
      uint nchan() const
        { return itsNChan; }
      uint startchan() const
        { return itsStartChan; }
      uint origNChan() const
        { return itsOrigNChan; }
      uint nchanAvg() const
        { return itsChanAvg; }
      uint nantenna() const
        { return itsAntNames.size(); }
      uint nbaselines() const
        { return itsAnt1.size(); }
      uint ntime() const
        { return itsNTime; }
      uint ntimeAvg() const
        { return itsTimeAvg; }
      double startTime() const
        { return itsStartTime; }
      double timeInterval() const
        { return itsTimeInterval; }
      const casa::Vector<casa::Int>& getAnt1() const
        { return itsAnt1; }
      const casa::Vector<casa::Int>& getAnt2() const
        { return itsAnt2; }
      const casa::Vector<casa::String>& antennaNames() const
        { return itsAntNames; }
      const casa::Vector<casa::Double>& antennaDiam() const
        { return itsAntDiam; }
      const vector<casa::MPosition>& antennaPos() const
        { return itsAntPos; }
      const casa::MPosition& arrayPos() const
        { return itsArrayPos; }
      const casa::MPosition arrayPosCopy() const
        {  return copyMeasure(casa::MeasureHolder(itsArrayPos)).asMPosition(); }
      const casa::MDirection& phaseCenter() const
        { return itsPhaseCenter; }
      const casa::MDirection phaseCenterCopy() const
      {  return copyMeasure(casa::MeasureHolder(itsPhaseCenter)).asMDirection(); }
      bool phaseCenterIsOriginal() const
        { return itsPhaseCenterIsOriginal; }
      const casa::MDirection& delayCenter() const
        { return itsDelayCenter; }
      const casa::MDirection delayCenterCopy() const
        { return copyMeasure(casa::MeasureHolder(itsDelayCenter)).asMDirection(); }
      const casa::MDirection& tileBeamDir() const
        { return itsTileBeamDir; }
      const casa::MDirection tileBeamDirCopy() const
        { return copyMeasure(casa::MeasureHolder(itsTileBeamDir)).asMDirection(); }
      const casa::Vector<double>& chanFreqs() const
        { return itsChanFreqs; }
      const casa::Vector<double>& chanWidths() const
        { return itsChanWidths; }
      const casa::Vector<double>& resolutions() const
        { return itsResolutions; }
      const casa::Vector<double>& effectiveBW() const
        { return itsEffectiveBW; }
      double totalBW() const
        { return itsTotalBW; }
      double refFreq() const
        { return itsRefFreq; }

      // Get the antenna numbers actually used in the (selected) baselines.
      // E.g. [0,2,5,6]
      const vector<int>& antennaUsed() const
        { return itsAntUsed; }

      // Get the indices of all antennae in the used antenna vector above.
      // -1 means that the antenna is not used.
      // E.g. [0,-1,1,-1,-1,2,3] for the example above.
      const vector<int>& antennaMap() const
        { return itsAntMap; }

      // Are the visibility data needed?
      bool needVisData() const
        { return itsNeedVisData; }
      // Does the last step need to write data and/or flags?
      int needWrite() const
        { return itsNeedWrite; }

      // Set if visibility data needs to be read.
      void setNeedVisData()
        { itsNeedVisData = true; } 
      // Set if the last step needs to write data and/or flags (default both).
      void setNeedWrite (int needWrite = NeedWriteData+NeedWriteFlags)
        { itsNeedWrite |= needWrite; }

      // Get the baseline table index of the autocorrelations.
      // A negative value means there are no autocorrelations for that antenna.
      const vector<int>& getAutoCorrIndex() const;

      // Get the lengths of the baselines (in meters).
      const vector<double>& getBaselineLengths() const;

      // Convert to a Record.
      // The names of the fields in the record are the data names without 'its'.
      casa::Record toRecord() const;

      // Update the DPInfo object from a Record.
      // It is possible that only a few fields are defined in the record.
      void fromRecord (const casa::Record& rec);

    private:
      // Set which antennae are actually used.
      void setAntUsed();

      // Creates a real copy of a casa::Measure by exporting to a Record
      static casa::MeasureHolder copyMeasure(const casa::MeasureHolder fromMeas);

      //# Data members.
      bool   itsNeedVisData;    //# Are the visibility data needed?
      int    itsNeedWrite;      //# Does the last step need to write data/flags?
      string itsMSName;
      string itsAntennaSet;
      uint   itsNCorr;
      uint   itsStartChan;
      uint   itsOrigNChan;
      uint   itsNChan;
      uint   itsChanAvg;
      uint   itsNTime;
      uint   itsTimeAvg;
      double itsStartTime;
      double itsTimeInterval;
      casa::MDirection itsPhaseCenter;
      bool             itsPhaseCenterIsOriginal;
      casa::MDirection itsDelayCenter;
      casa::MDirection itsTileBeamDir;
      casa::MPosition  itsArrayPos;
      casa::Vector<double>       itsChanFreqs;
      casa::Vector<double>       itsChanWidths;
      casa::Vector<double>       itsResolutions;
      casa::Vector<double>       itsEffectiveBW;
      double                     itsTotalBW;
      double                     itsRefFreq;
      casa::Vector<casa::String> itsAntNames;
      casa::Vector<casa::Double> itsAntDiam;
      vector<casa::MPosition>    itsAntPos;
      vector<int>                itsAntUsed;
      vector<int>                itsAntMap;
      casa::Vector<casa::Int>    itsAnt1;          //# ant1 of all baselines
      casa::Vector<casa::Int>    itsAnt2;          //# ant2 of all baselines
      mutable vector<double>     itsBLength;       //# baseline lengths
      mutable vector<int>        itsAutoCorrIndex; //# autocorr index per ant
    };

  } //# end namespace
}

#endif
