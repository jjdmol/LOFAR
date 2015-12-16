//# Step.cc:
//#
//# Copyright (C) 2002-2007
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
//#  $Id$

#include <lofar_config.h>

#include <BBSControl/Step.h>
#include <BBSControl/CorrectStep.h>
#include <BBSControl/PredictStep.h>
#include <BBSControl/RefitStep.h>
#include <BBSControl/ShiftStep.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/SubtractStep.h>
#include <BBSControl/AddStep.h>
#include <BBSControl/MultiStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/StreamUtil.h>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{

  namespace BBS
  {
    using LOFAR::operator<<;

    //##--------   P u b l i c   m e t h o d s   --------##//

    Step::~Step()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    string Step::fullName() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      string name;
      if (itsParent) name = itsParent->fullName() + ".";
      name += itsName;
      return name;
    }


    shared_ptr<Step> Step::create(const string& name,
                                        const ParameterSet& parset,
                                        const Step* parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      shared_ptr<Step> step;

      // If \a parset contains a key <tt>Step.<em>name</em>.Steps</tt>, then
      // \a name is a MultiStep, otherwise it is a SingleStep.
      if (parset.isDefined("Step." + name + ".Steps")) {
        LOG_TRACE_COND_STR(name << " is a MultiStep");
        step.reset(new MultiStep(name, parset, parent));
      } else {
        LOG_TRACE_COND_STR(name << " is a SingleStep");
        // We'll have to figure out what kind of SingleStep we must
        // create. The key "Operation" contains this information.
        try {
          string oper =
            toUpper(parset.getString("Step." + name + ".Operation"));
          LOG_TRACE_COND_STR("Creating a " << oper << " step ...");
          if      (oper == "SOLVE")
            step.reset(new SolveStep(name, parset, parent));
          else if (oper == "SUBTRACT")
            step.reset(new SubtractStep(name, parset, parent));
          else if (oper == "ADD")
            step.reset(new AddStep(name, parset, parent));
          else if (oper == "CORRECT")
            step.reset(new CorrectStep(name, parset, parent));
          else if (oper == "PREDICT")
            step.reset(new PredictStep(name, parset, parent));
          else if (oper == "SHIFT")
            step.reset(new ShiftStep(name, parset, parent));
          else if (oper == "REFIT")
            step.reset(new RefitStep(name, parset, parent));
          else THROW (BBSControlException, "Operation \"" << oper <<
                      "\" is not a valid Step operation");
         } catch (APSException& e) {
          THROW (BBSControlException, e.what());
        }
      }
      return step;
    }


    //##--------   P r o t e c t e d   m e t h o d s   --------##//

    Step::Step(const string& name, const Step* parent)
        : itsBaselines("*&")
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Copy the data members from the parent, if present, so that they have
      // sensible default values.
      if (parent) *this = *parent;

      // We must reset these values, because they were overwritten by the
      // previous data copy.
      itsName = name;
      itsParent = parent;
    }


    void Step::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_COND, "Step." << itsName);
      const string prefix = "Step." + itsName + ".";

      // Write data selection.
      ps.replace(prefix + "Baselines", itsBaselines);
      ps.replace(prefix + "Correlations", toString(itsCorrelations));

      // Remove all Model keys from the parset. Effectively, key inheritance
      // is handled in read() and here we only output the keys that are relevant
      // for this Step.
      ps.subtractSubset(prefix + "Model.");

      ps.add(prefix + "Model.Bandpass.Enable",
        toString(itsModelConfig.useBandpass()));
      ps.add(prefix + "Model.Clock.Enable",
        toString(itsModelConfig.useClock()));
      if (itsModelConfig.useClock()) {
        const ClockConfig &config = itsModelConfig.getClockConfig();
        ps.add(prefix + "Model.Clock.Split", toString(config.splitClock()));
      }

      ps.add(prefix + "Model.Gain.Enable",
        toString(itsModelConfig.useGain()));
      if(itsModelConfig.useGain()) {
        ps.add(prefix + "Model.Gain.Phasors",
          toString(itsModelConfig.getGainConfig().phasors()));
      }

      ps.add(prefix + "Model.TEC.Enable",
        toString(itsModelConfig.useTEC()));
      ps.add(prefix + "Model.CommonRotation.Enable",
        toString(itsModelConfig.useCommonRotation()));
      ps.add(prefix + "Model.CommonScalarPhase.Enable",
        toString(itsModelConfig.useCommonScalarPhase()));

      ps.add(prefix + "Model.DirectionalGain.Enable",
        toString(itsModelConfig.useDirectionalGain()));
      if(itsModelConfig.useDirectionalGain()) {
        ps.add(prefix + "Model.DirectionalGain.Phasors",
          toString(itsModelConfig.getDirectionalGainConfig().phasors()));
      }

      ps.add(prefix + "Model.Beam.Enable", toString(itsModelConfig.useBeam()));
      if(itsModelConfig.useBeam()) {
        const BeamConfig &config = itsModelConfig.getBeamConfig();
        ps.add(prefix + "Model.Beam.Mode", BeamConfig::asString(config.mode()));
        ps.add(prefix + "Model.Beam.UseChannelFreq",
          toString(config.useChannelFreq()));
      }

      ps.add(prefix + "Model.DirectionalTEC.Enable",
        toString(itsModelConfig.useDirectionalTEC()));
      ps.add(prefix + "Model.FaradayRotation.Enable",
        toString(itsModelConfig.useFaradayRotation()));
      ps.add(prefix + "Model.Rotation.Enable",
        toString(itsModelConfig.useRotation()));
      ps.add(prefix + "Model.ScalarPhase.Enable",
        toString(itsModelConfig.useScalarPhase()));

      ps.add(prefix + "Model.Ionosphere.Enable",
        toString(itsModelConfig.useIonosphere()));
      if(itsModelConfig.useIonosphere()) {
        const IonosphereConfig &config = itsModelConfig.getIonosphereConfig();
        ps.add(prefix + "Model.Ionosphere.Type",
          IonosphereConfig::asString(config.getModelType()));
        if(config.getModelType() == IonosphereConfig::MIM) {
          ps.add(prefix + "Model.Ionosphere.Degree",
            toString(config.degree()));
        }
      }

      ps.add(prefix + "Model.Flagger.Enable",
        toString(itsModelConfig.useFlagger()));
      if(itsModelConfig.useFlagger()) {
        const FlaggerConfig &config = itsModelConfig.getFlaggerConfig();
        ps.add(prefix + "Model.Flagger.Threshold",
          toString(config.threshold()));
      }

      ps.add(prefix + "Model.Cache.Enable",
        toString(itsModelConfig.useCache()));

      ps.add(prefix + "Model.Sources", toString(itsModelConfig.sources()));

      LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
    }


    void Step::read(const ParameterSet& ps, const std::string prefix)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Read data selection.
      itsBaselines = ps.getString(prefix+"Baselines", itsBaselines);
      itsCorrelations = ps.getStringVector(prefix+"Correlations", itsCorrelations);

      // Read model configuration.
      bool usePhasors = ps.getBool(prefix+"Model.Phasors.Enable", false);

      itsModelConfig.setBandpass(ps.getBool(prefix+"Model.Bandpass.Enable",
        itsModelConfig.useBandpass()));

      if (ps.getBool(prefix+"Model.Clock.Enable", itsModelConfig.useClock())) {
        ClockConfig parentConfig = itsModelConfig.getClockConfig();

        bool splitClock = false;
        if(itsModelConfig.useClock()) {
          splitClock = ps.getBool(prefix+"Model.Clock.Split",
              parentConfig.splitClock());
        } else {
          splitClock = ps.getBool(prefix+"Model.Clock.Split", false);
        }
        itsModelConfig.setClockConfig(ClockConfig(splitClock));
      } else {
        itsModelConfig.clearClockConfig();
      }

      if(ps.getBool(prefix+"Model.Gain.Enable", itsModelConfig.useGain())) {
        const GainConfig &parentConfig = itsModelConfig.getGainConfig();
        bool phasors = ps.getBool(prefix+"Model.Gain.Phasors",
            itsModelConfig.useGain() ? parentConfig.phasors() : usePhasors);
        itsModelConfig.setGainConfig(GainConfig(phasors));
      }
      else {
        itsModelConfig.clearGainConfig();
      }

      if (ps.getBool(prefix+"Model.TEC.Enable", itsModelConfig.useTEC())) {
        TECConfig parentConfig = itsModelConfig.getTECConfig();

        bool splitTEC = false;
        if(itsModelConfig.useTEC()) {
          splitTEC = ps.getBool(prefix+"Model.TEC.Split",
              parentConfig.splitTEC());
        } else {
          splitTEC = ps.getBool(prefix+"Model.TEC.Split", false);
        }
        itsModelConfig.setTECConfig(TECConfig(splitTEC));
      } else {
        itsModelConfig.clearTECConfig();
      }

      itsModelConfig.setCommonRotation(ps.getBool(prefix+"Model.CommonRotation.Enable",
        itsModelConfig.useCommonRotation()));

      itsModelConfig.setCommonScalarPhase
        (ps.getBool(prefix+"Model.CommonScalarPhase.Enable",
          itsModelConfig.useCommonScalarPhase()));

      if(ps.getBool(prefix+"Model.DirectionalGain.Enable",
        itsModelConfig.useDirectionalGain())) {

        const DirectionalGainConfig &parentConfig =
          itsModelConfig.getDirectionalGainConfig();
        bool phasors = ps.getBool(prefix+"Model.DirectionalGain.Phasors",
          itsModelConfig.useDirectionalGain() ? parentConfig.phasors()
          : usePhasors);

        itsModelConfig.setDirectionalGainConfig(DirectionalGainConfig(phasors));
      }
      else {
        itsModelConfig.clearDirectionalGainConfig();
      }

      if(ps.getBool(prefix+"Model.Beam.Enable", itsModelConfig.useBeam())) {
        const BeamConfig &parentConfig = itsModelConfig.getBeamConfig();

        string modeString = ps.getString(prefix+"Model.Beam.Mode",
          itsModelConfig.useBeam() ? BeamConfig::asString(parentConfig.mode())
            : BeamConfig::asString(BeamConfig::DEFAULT));
        BeamConfig::Mode mode = BeamConfig::asMode(modeString);
        if(!BeamConfig::isDefined(mode)) {
          THROW(BBSControlException, "Key "+prefix+"Model.Beam.Mode invalid.");
        }

        bool useChannelFreq = ps.getBool(prefix+"Model.Beam.UseChannelFreq",
          itsModelConfig.useBeam() ? parentConfig.useChannelFreq() : false);
        bool conjugateAF = ps.getBool(prefix+"Model.Beam.ConjugateAF", false);
        if(conjugateAF)
        {
          THROW(BBSControlException, "The key Step.*.Model.Beam.ConjugateAF has"
            " been deprecated and should not be used.");
        }

        itsModelConfig.setBeamConfig(BeamConfig(mode, useChannelFreq));
      } else {
        itsModelConfig.clearBeamConfig();
      }

      itsModelConfig.setDirectionalTEC(ps.getBool(prefix+"Model.DirectionalTEC.Enable",
        itsModelConfig.useDirectionalTEC()));

      itsModelConfig.setFaradayRotation
        (ps.getBool(prefix+"Model.FaradayRotation.Enable",
          itsModelConfig.useFaradayRotation()));

      itsModelConfig.setRotation(ps.getBool(prefix+"Model.Rotation.Enable",
        itsModelConfig.useRotation()));

      itsModelConfig.setScalarPhase(ps.getBool(prefix+"Model.ScalarPhase.Enable",
        itsModelConfig.useScalarPhase()));

      if(ps.getBool(prefix+"Model.Ionosphere.Enable", itsModelConfig.useIonosphere()))
      {
        const IonosphereConfig &parentConfig =
            itsModelConfig.getIonosphereConfig();

        string modelTypeString;
        if(itsModelConfig.useIonosphere()) {
          modelTypeString = ps.getString(prefix+"Model.Ionosphere.Type",
            IonosphereConfig::asString(parentConfig.getModelType()));
        } else {
          modelTypeString = ps.getString(prefix+"Model.Ionosphere.Type");
        }

        IonosphereConfig::ModelType modelType =
          IonosphereConfig::asModelType(modelTypeString);
        if(!IonosphereConfig::isDefined(modelType)) {
          THROW(BBSControlException, "Key Model.Ionosphere.Type not found or"
            " invalid.");
        }

        unsigned int degree = 0;
        if(itsModelConfig.useIonosphere()
          && parentConfig.getModelType() == IonosphereConfig::MIM) {
          degree = ps.getUint(prefix+"Model.Ionosphere.Degree", parentConfig.degree());
        } else if(modelType == IonosphereConfig::MIM) {
          degree = ps.getUint(prefix+"Model.Ionosphere.Degree");
        }

        itsModelConfig.setIonosphereConfig(IonosphereConfig(modelType, degree));
      } else {
        itsModelConfig.clearIonosphereConfig();
      }

      if(ps.getBool(prefix+"Model.Flagger.Enable", itsModelConfig.useFlagger())) {
        double threshold = 0.0;
        if(itsModelConfig.useFlagger()) {
          threshold = ps.getDouble(prefix+"Model.Flagger.Threshold",
            itsModelConfig.getFlaggerConfig().threshold());
        } else {
          threshold = ps.getDouble(prefix+"Model.Flagger.Threshold");
        }

        itsModelConfig.setFlaggerConfig(FlaggerConfig(threshold));
      } else {
        itsModelConfig.clearFlaggerConfig();
      }

      itsModelConfig.setCache(ps.getBool(prefix+"Model.Cache.Enable",
        itsModelConfig.useCache()));

      itsModelConfig.setSources(ps.getStringVector(prefix+"Model.Sources",
        itsModelConfig.sources()));
    }


    void Step::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      os << "Step: " << type();
      Indent id;  // add an extra indentation level
      os << endl << indent << "Name: " << itsName
        << endl << indent << "Full name: " << fullName()
        << endl << indent << "Baselines: " << itsBaselines
        << endl << indent << "Correlations: " << itsCorrelations
        << endl << indent << itsModelConfig;
    }


    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const Step& bs)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      bs.print(os);
      return os;
    }


  } // namespace BBS

} // namespace LOFAR
