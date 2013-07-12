//# DemixInfo.cc: Struct to hold the common demix variables
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

#include <lofar_config.h>
#include <DPPP/DemixInfo.h>
#include <DPPP/PointSource.h>
#include <DPPP/GaussianSource.h>
#include <DPPP/Stokes.h>
#include <ParmDB/SourceDB.h>
#include <Common/ParameterSet.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>

using namespace casa;

namespace LOFAR {
  namespace DPPP {

    DemixInfo::DemixInfo (const ParameterSet& parset, const string& prefix)
      : itsSelBL            (parset, prefix, false, "cross"),
        itsSelBLRatio       (parset, prefix+"ratio.", false, "cross"), 
        itsPredictModelName (parset.getString(prefix+"skymodelPredict")),
        itsDemixModelName   (parset.getString(prefix+"skymodelDemix")),
        itsTargetModelName  (parset.getString(prefix+"skymodelTarget")),
        itsSourceNames      (parset.getStringVector (prefix+"sources")),
        itsRatioPattern     (parset.getString (prefix+"ratioBaselines", "CS*&")),
        itsRatio1           (parset.getDouble (prefix+"ratio1", 5.)),
        itsRatio2           (parset.getDouble (prefix+"ratio2", 0.25)),
        itsAmplThreshold    (parset.getDouble (prefix+"amplThreshold", 20.)),
        itsAngdistThreshold (parset.getDouble (prefix+"distanceThreshold", 60.)),
        itsAngdistRefFreq   (parset.getDouble (prefix+"distanceRefFreq", 60e6)),
        itsMinNStation      (parset.getDouble (prefix+"minnstation", 6)),
        itsNStation         (0),
        itsNBl              (0),
        itsNCorr            (0),
        itsNChanIn          (0),
        itsNChanAvgSubtr    (parset.getUint  (prefix+"freqstep", 1)),
        itsNChanAvg         (parset.getUint  (prefix+"demixfreqstep",
                                              itsNChanAvgSubtr)),
        itsNChanOutSubtr    (0),
        itsNChanOut         (0),
        itsNTimeAvgSubtr    (parset.getUint  (prefix+"timestep", 1)),
        itsNTimeAvg         (parset.getUint  (prefix+"demixtimestep",
                                              itsNTimeAvgSubtr)),
        itsChunkSize        (parset.getUint  (prefix+"chunksize", 120)),
        itsNTimeChunk       (parset.getUint  (prefix+"ntimechunk", 0)),
        itsTimeIntervalAvg  (0)
    {
      // Get delta in arcsec and take cosine of it (convert to radians first).
      double delta = parset.getDouble (prefix+"distanceDelta", 60.);
      itsCosAngdistDelta = cos (delta / 3600. * casa::C::pi / 180.);
      ASSERTSTR (!(itsPredictModelName.empty() || itsDemixModelName.empty() ||
                   itsTargetModelName.empty()),
                 "An empty name is given for a sky model");
      vector<Patch::ConstPtr> ateamList = makePatchList (itsPredictModelName,
                                                         itsSourceNames);
      itsAteamDemixList = makePatchList (itsDemixModelName, itsSourceNames);
      itsTargetList     = makePatchList (itsTargetModelName, vector<string>());
      ASSERT (ateamList.size() == itsAteamDemixList.size());
      // Make sure the Ateam models are in the same order and having matching
      // positions.
      itsAteamList.reserve (ateamList.size());
      for (size_t i=0; i<itsAteamDemixList.size(); ++i) {
        for (size_t j=0; j<ateamList.size(); ++j) {
          if (ateamList[j]->name() == itsAteamDemixList[i]->name()) {
            ASSERTSTR (testAngDist (itsAteamDemixList[i]->position()[0],
                                    itsAteamDemixList[i]->position()[1],
                                    ateamList[j]->position()[0],
                                    ateamList[j]->position()[1],
                                    itsCosAngdistDelta),
                       "Position mismatch of source " << ateamList[j]->name()
                       << " in Ateam SourceDBs (["
                       << itsAteamDemixList[i]->position()[0] << ", "
                       << itsAteamDemixList[i]->position()[1] << "] and ["
                       << ateamList[j]->position()[0] << ", "
                       << ateamList[j]->position()[1] << "])");
            itsAteamList.push_back (ateamList[j]);
            break;
          }
        }
      }
      // Make a more detailed target model in case it contains A-team sources.
      makeTargetDemixList();
    }

    void DemixInfo::makeTargetDemixList()
    {
      // Get all Ateam models for demixing.
      // Open the SourceDB.
      BBS::SourceDB sdb(BBS::ParmDBMeta(string(), itsDemixModelName));
      sdb.lock();
      vector<Patch::ConstPtr> patchList = makePatchList (itsDemixModelName,
                                                         vector<string>());
      // The demix target list is the same as the predict list, but Ateam
      // sources must be replaced with their demix model.
      // Also these sources must be removed from the Ateam model.
      itsTargetDemixList.reserve (itsTargetList.size());
      for (size_t i=0; i<itsTargetList.size(); ++i) {
        // Initially use rough target model.
        itsTargetDemixList.push_back (itsTargetList[i]);
        // Look if an Ateam source matches this target source.
        for (size_t j=0; j<patchList.size(); ++j) {
          if (testAngDist (itsTargetList[i]->position()[0],
                           itsTargetList[i]->position()[1],
                           patchList[j]->position()[0],
                           patchList[j]->position()[1],
                           itsCosAngdistDelta)) {
            // Match, so use the detailed Ateam model.
            itsTargetDemixList[i] = patchList[j];
            // A-source is in target, so remove from Ateam models (if in there).
            for (size_t k=0; k<itsAteamList.size(); ++k) {
              if (testAngDist (itsTargetDemixList[i]->position()[0],
                               itsTargetDemixList[i]->position()[1],
                               itsAteamList[k]->position()[0],
                               itsAteamList[k]->position()[1],
                               itsCosAngdistDelta)) {
                itsAteamList.erase (itsAteamList.begin() + k);
                itsAteamDemixList.erase (itsAteamDemixList.begin() + k);
                break;
              }
            }
            break;
          }
        }
      }
    }
 
    void DemixInfo::update (const DPInfo& infoSel, DPInfo& info)
    {
      // Get size info.
      itsNChanIn = infoSel.nchan();
      itsNCorr   = infoSel.ncorr();
      ASSERTSTR (itsNCorr==4, "Demixing requires data with 4 polarizations");

      // NB. The number of baselines and stations refer to the number of
      // selected baselines and the number of unique stations participating
      // in the selected baselines.
      itsNBl = infoSel.nbaselines();
      itsNStation = infoSel.antennaUsed().size();

      // Setup the baseline index vector used to split the UVWs.
      itsUVWSplitIndex = setupSplitUVW (infoSel.nantenna(),
                                        infoSel.getAnt1(), infoSel.getAnt2());

      // Determine which baselines to use when determining ratio target/Ateam.
      ParameterSet pset;
      pset.add ("baseline", itsRatioPattern);
      itsSelRatio = BaselineSelection(pset, string(), false, "cross");
      itsSelRatio.apply (infoSel);

      // Re-number the station IDs in the selected baselines, removing gaps in
      // the numbering due to unused stations.
      const vector<int> &antennaMap = infoSel.antennaMap();
      for (uint i=0; i<itsNBl; ++i) {
        itsBaselines.push_back(Baseline(antennaMap[infoSel.getAnt1()[i]],
          antennaMap[infoSel.getAnt2()[i]]));
      }

      // Adapt averaging to available nr of channels and times.
      // Use a copy of the DPInfo, otherwise it is updated multiple times.
      DPInfo infoDemix(infoSel);
      itsNTimeAvg = std::min (itsNTimeAvg, infoSel.ntime());
      itsNChanAvg = infoDemix.update (itsNChanAvg, itsNTimeAvg);
      itsNChanOut = infoDemix.nchan();
      itsTimeIntervalAvg = infoDemix.timeInterval();
      ///      itsNTimeDemix      = infoDemix.ntime();

      itsNTimeAvgSubtr = std::min (itsNTimeAvgSubtr, infoSel.ntime());
      itsNChanAvgSubtr = info.update (itsNChanAvgSubtr, itsNTimeAvgSubtr);
      itsNChanOutSubtr = info.nchan();
      ASSERTSTR (itsNChanAvg % itsNChanAvgSubtr == 0,
        "Demix frequency averaging " << itsNChanAvg
        << " must be a multiple of output averaging "
        << itsNChanAvgSubtr);
      ASSERTSTR (itsNTimeAvg % itsNTimeAvgSubtr == 0,
        "Demix time averaging " << itsNTimeAvg
        << " must be a multiple of output averaging "
        << itsNTimeAvgSubtr);
      ASSERTSTR (itsChunkSize % itsNTimeAvg == 0,
        "Demix time chunk size " << itsChunkSize
        << " must be a multiple of demix time averaging "
        << itsNTimeAvg);
      itsNTimeOut = itsChunkSize / itsNTimeAvg;
      itsNTimeOutSubtr = itsChunkSize / itsNTimeAvgSubtr;
      // Store channel frequencies for the demix and subtract resolutions.
      itsFreqDemix = infoDemix.chanFreqs();
      itsFreqSubtr = info.chanFreqs();

      // Store phase center position in J2000.
      MDirection dirJ2000(MDirection::Convert(infoSel.phaseCenter(),
                                              MDirection::J2000)());
      Quantum<Vector<Double> > angles = dirJ2000.getAngle();
      itsPhaseRef = Position(angles.getBaseValue()[0],
                             angles.getBaseValue()[1]);
    }

    void DemixInfo::show (ostream& os) const
    {
      os << "  freqstep:           " << itsNChanAvgSubtr << endl;
      os << "  timestep:           " << itsNTimeAvgSubtr << endl;
      os << "  demixfreqstep:      " << itsNChanAvg << endl;
      os << "  demixtimestep:      " << itsNTimeAvg << endl;
      os << "  chunksize:          " << itsChunkSize << endl;
      os << "  ntimechunk:         " << itsNTimeChunk << endl;
      itsSelBL.show (os);
    }

    vector<Patch::ConstPtr>
    DemixInfo::makePatchList (const string& sdbName,
                              const vector<string>& patchNames)
    {
      // Open the SourceDB.
      BBS::SourceDB sdb(BBS::ParmDBMeta(string(), sdbName));
      sdb.lock();
      // Get all patches from it.
      vector<string> names(sdb.getPatches());
      vector<string>::const_iterator pnamesIter = patchNames.begin();
      vector<string>::const_iterator pnamesEnd  = patchNames.end();
      if (patchNames.empty()) {
        pnamesIter = names.begin();
        pnamesEnd  = names.end();
      }
      // Create a patch component list for each matching patch name.
      vector<Patch::ConstPtr> patchList;
      patchList.reserve (patchNames.size());
      for (; pnamesIter != pnamesEnd; ++pnamesIter) {
        ASSERTSTR (std::find (names.begin(), names.end(), *pnamesIter)
                   != names.end(),
                   "Demixer: sourcename " << *pnamesIter
                   << " not found in SourceDB " << sdbName);
        // Use this patch; get all its sources.
        vector<BBS::SourceData> patch = sdb.getPatchSourceData (*pnamesIter);
        vector<ModelComponent::Ptr> componentList;
        componentList.reserve (patch.size());
        for (vector<BBS::SourceData>::const_iterator iter=patch.begin();
             iter!=patch.end(); ++iter) {
          const BBS::SourceData& src = *iter;
          // Fetch position.
          ASSERT (src.getInfo().getRefType() == "J2000");
          Position position;
          position[0] = src.getRa();
          position[1] = src.getDec();

          // Fetch stokes vector.
          Stokes stokes;
          stokes.I = src.getI();
          stokes.V = src.getV();
          if (!src.getInfo().getUseRotationMeasure()) {
            stokes.Q = src.getQ();
            stokes.U = src.getU();
          }

          PointSource::Ptr source;
          switch (src.getInfo().getType()) {
          case BBS::SourceInfo::POINT:
            {
              source = PointSource::Ptr(new PointSource(position, stokes));
            }
            break;
          case BBS::SourceInfo::GAUSSIAN:
            {
              GaussianSource::Ptr gauss(new GaussianSource(position, stokes));
              const double deg2rad = (casa::C::pi / 180.0);
              gauss->setPositionAngle(src.getOrientation() * deg2rad);
              const double arcsec2rad = (casa::C::pi / 3600.0) / 180.0;
              gauss->setMajorAxis(src.getMajorAxis() * arcsec2rad);
              gauss->setMinorAxis(src.getMinorAxis() * arcsec2rad);
              source = gauss;
            }
            break;
          default:
            {
              ASSERTSTR(false, "Only point sources and Gaussian sources are"
                        " supported at this time.");
            }
          }

          // Fetch spectral index attributes (if applicable).
          if (src.getSpectralIndex().size() > 0) {
            source->setSpectralIndex(src.getInfo().getSpectralIndexRefFreq(),
                                     src.getSpectralIndex().begin(),
                                     src.getSpectralIndex().end());
          }

          // Fetch rotation measure attributes (if applicable).
          if (src.getInfo().getUseRotationMeasure()) {
            source->setRotationMeasure(src.getPolarizedFraction(),
                                       src.getPolarizationAngle(),
                                       src.getRotationMeasure());
          }

          // Add the source definition.
          componentList.push_back(source);
        }

        // Add the component list to the list of patches.
        patchList.push_back (Patch::Ptr (new Patch(*pnamesIter,
                                                   componentList.begin(),
                                                   componentList.end())));
      }
      return patchList;
    }

    bool DemixInfo::testAngDist (double ra1, double dec1,
                                 double ra2, double dec2,
                                 double cosDelta)
    {
      return sin(dec1)*sin(dec2) + cos(dec1)*cos(dec2)*cos(ra1-ra2) >= cosDelta;
    }


  } //# end namespace
} //# end namespace
