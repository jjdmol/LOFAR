//# DemixInfo.h: Struct to hold the common demix variables
//# Copyright (C) 2013
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
//# $Id: Demixer.h 23223 2012-12-07 14:09:42Z schoenmakers $
//#
//# @author Ger van Diepen

#ifndef DPPP_DEMIXINFO_H
#define DPPP_DEMIXINFO_H

// @file
// @brief DPPP step class to average in time and/or freq

#include <DPPP/Baseline.h>
#include <DPPP/DPInput.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/Patch.h>
#include <DPPP/PhaseShift.h>
#include <DPPP/Filter.h>

#include <casa/Arrays/Cube.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>

namespace LOFAR {
  namespace DPPP {
    // @ingroup NDPPP

    // This struct holds the common demix variables.
    // It can be shared between the parallel DemixWorker objects.

    class DemixInfo
    {
    public:
      // Constructor to read and initialize the values.
      DemixInfo (const ParameterSet&, const string& prefix);

      // Update the info.
      void update (const DPInfo& infoSel, DPInfo& info);

      // Show parameters.
      void show (ostream&) const;

      //# Data members
      uint   minNStation() const                   {return itsMinNStation;}
      uint   nstation() const                      {return itsNStation;}
      uint   nbl() const                           {return itsNBl;}
      uint   ncorr() const                         {return itsNCorr;}
      uint   nchanIn() const                       {return itsNChanIn;}
      uint   nchanAvg() const                      {return itsNChanAvg;}
      uint   nchanAvgSubtr() const                 {return itsNChanAvgSubtr;}
      uint   nchanOut() const                      {return itsNChanOut;}
      uint   nchanOutSubtr() const                 {return itsNChanOutSubtr;}
      uint   ntimeAvg() const                      {return itsNTimeAvg;}
      uint   ntimeAvgSubtr() const                 {return itsNTimeAvgSubtr;}
      uint   ntimeOut() const                      {return itsNTimeOut;}
      uint   ntimeOutSubtr() const                 {return itsNTimeOutSubtr;}
      uint   ntimeChunk() const                    {return itsNTimeChunk;}
      uint   chunkSize() const                     {return itsChunkSize;}
      uint   timeWindow() const      {return itsNTimeChunk*itsChunkSize;}
      double timeIntervalAvg() const               {return itsTimeIntervalAvg;}
      double ratio1() const                        {return itsRatio1;}
      double ratio2() const                        {return itsRatio2;}
      double amplThreshold() const                 {return itsAmplThreshold;}
      double angdistThreshold() const              {return itsAngdistThreshold;}
      double angdistRefFreq() const                {return itsAngdistRefFreq;}
      const BaselineSelection& selBL() const       {return itsSelBL;}
      const BaselineSelection& selBLRatio() const  {return itsSelBLRatio;}
      const vector<uint>& uvwSplitIndex() const    {return itsUVWSplitIndex;}
      const string& predictModelName() const       {return itsPredictModelName;}
      const string& demixModelName() const         {return itsDemixModelName;}
      const string& targetModelName() const        {return itsTargetModelName;}
      const vector<string>& sourceNames() const    {return itsSourceNames;}
      const Position& phaseRef() const             {return itsPhaseRef;}
      const vector<Baseline>& baselines() const    {return itsBaselines;}
      const casa::Vector<double>& freqDemix() const      {return itsFreqDemix;}
      const casa::Vector<double>& freqSubtr() const      {return itsFreqSubtr;}
      const vector<Patch::ConstPtr>& ateamList() const   {return itsAteamList;}
      const vector<Patch::ConstPtr>& targetList() const  {return itsTargetList;}
      const vector<Patch::ConstPtr>& ateamDemixList() const
        {return itsAteamDemixList;}
      const vector<Patch::ConstPtr>& targetDemixList() const
        {return itsTargetDemixList;}

      // Test if two positions in the sky are within delta radians.
      static bool testAngDist (double ra1, double dec1,
                               double ra2, double dec2, double cosDelta);

    private:
      // Create a list of patches (and components).
      vector<Patch::ConstPtr> makePatchList (const string& sdbName,
                                             const vector<string>& patchNames);

      // Make the target list for demixing with a detailed model for the
      // possible Ateam sources in it.
      void makeTargetDemixList();

      //# Data members.
      BaselineSelection       itsSelBL;
      BaselineSelection       itsSelBLRatio;
      vector<uint>            itsUVWSplitIndex;
      string                  itsPredictModelName;
      string                  itsDemixModelName;
      string                  itsTargetModelName;
      vector<string>          itsSourceNames;
      string                  itsRatioPattern;
      double                  itsCosAngdistDelta;
      double                  itsRatio1;
      double                  itsRatio2;
      double                  itsAmplThreshold;
      double                  itsAngdistThreshold;
      double                  itsAngdistRefFreq;
      uint                    itsMinNStation;        //# min #stations for solve
      uint                    itsNStation;
      uint                    itsNBl;
      uint                    itsNCorr;
      uint                    itsNChanIn;
      uint                    itsNChanAvgSubtr;      //# subtract averaging
      uint                    itsNChanAvg;           //# demix averaging
      uint                    itsNChanOutSubtr;
      uint                    itsNChanOut;
      uint                    itsNTimeAvgSubtr;      //# subtract averaging
      uint                    itsNTimeAvg;           //# demix averaging
      uint                    itsChunkSize;          //# nr times per chunk
      uint                    itsNTimeOutSubtr;      //# #output times per chunk
      uint                    itsNTimeOut;           //# #demix times per chunk
      uint                    itsNTimeChunk;         //# nr chunks in parallel
      double                  itsTimeIntervalAvg;
      Position                itsPhaseRef;           //# original phaseref
      vector<Baseline>        itsBaselines;
      casa::Vector<double>    itsFreqDemix;
      casa::Vector<double>    itsFreqSubtr;
      vector<Patch::ConstPtr> itsAteamList;
      vector<Patch::ConstPtr> itsTargetList;
      vector<Patch::ConstPtr> itsAteamDemixList;
      vector<Patch::ConstPtr> itsTargetDemixList;
    };

  } //# end namespace
} //# end namespace

#endif
