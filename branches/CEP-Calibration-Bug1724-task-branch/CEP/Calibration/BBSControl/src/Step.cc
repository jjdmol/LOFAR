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

      ps.add(prefix + "Model.Gain.Enable",
        toString(itsModelConfig.useGain()));
      if(itsModelConfig.useGain()) {
        ps.add(prefix + "Model.Gain.Phasors",
          toString(itsModelConfig.getGainConfig().phasors()));
      }

      ps.add(prefix + "Model.TEC.Enable",
        toString(itsModelConfig.useTEC()));

      ps.add(prefix + "Model.DirectionalGain.Enable",
        toString(itsModelConfig.useDirectionalGain()));
      if(itsModelConfig.useDirectionalGain()) {
        ps.add(prefix + "Model.DirectionalGain.Patches",
            itsPartitionDirectionalGain);
        ps.add(prefix + "Model.DirectionalGain.Phasors",
            toString(itsModelConfig.getDirectionalGainConfig().phasors()));
      }

      ps.add(prefix + "Model.Beam.Enable", toString(itsModelConfig.useBeam()));
      if(itsModelConfig.useBeam()) {
        ps.add(prefix + "Model.Beam.Patches", itsPartitionBeam);

        const BeamConfig &config = itsModelConfig.getBeamConfig();
        ps.add(prefix + "Model.Beam.Mode", BeamConfig::asString(config.mode()));
        ps.add(prefix + "Model.Beam.ConjugateAF",
          toString(config.conjugateAF()));
        ps.add(prefix + "Model.Beam.Element.Path",
          config.getElementPath().originalName());
      }

      ps.add(prefix + "Model.DirectionalTEC.Enable",
        toString(itsModelConfig.useDirectionalTEC()));
      if(itsModelConfig.useDirectionalTEC()) {
        ps.add(prefix + "Model.DirectionalTEC.Patches",
            itsPartitionDirectionalTEC);
      }

      ps.add(prefix + "Model.FaradayRotation.Enable",
        toString(itsModelConfig.useFaradayRotation()));
      if(itsModelConfig.useFaradayRotation()) {
        ps.add(prefix + "Model.FaradayRotation.Patches",
            itsPartitionFaradayRotation);
      }

      ps.add(prefix + "Model.Ionosphere.Enable",
        toString(itsModelConfig.useIonosphere()));
      if(itsModelConfig.useIonosphere()) {
        ps.add(prefix + "Model.Ionosphere.Patches", itsPartitionIonosphere);

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

      ps.add(prefix + "Model.Sources", toString(itsModelConfig.getSources()));

      LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
    }


    void Step::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Read data selection.
      itsBaselines = ps.getString("Baselines", itsBaselines);
      itsCorrelations = ps.getStringVector("Correlations", itsCorrelations);

      // Read model configuration.
      itsModelConfig.setBandpass(ps.getBool("Model.Bandpass.Enable",
        itsModelConfig.useBandpass()));

      itsModelConfig.setClock(ps.getBool("Model.Clock.Enable",
        itsModelConfig.useClock()));

      if(ps.getBool("Model.Gain.Enable", itsModelConfig.useGain())) {
        bool phasors = ps.getBool("Model.Gain.Phasors",
          itsModelConfig.getGainConfig().phasors());
        itsModelConfig.setGainConfig(GainConfig(phasors));
      }
      else {
        itsModelConfig.clearGainConfig();
      }

      itsModelConfig.setTEC(ps.getBool("Model.TEC.Enable",
        itsModelConfig.useTEC()));

      if(ps.getBool("Model.DirectionalGain.Enable",
        itsModelConfig.useDirectionalGain())) {

        const DirectionalGainConfig &parentConfig =
          itsModelConfig.getDirectionalGainConfig();

        string defaultPartition("[*]");
        if(itsModelConfig.useDirectionalGain()) {
            defaultPartition = itsPartitionDirectionalGain;
        }
        itsPartitionDirectionalGain =
            ps.getString("Model.DirectionalGain.Patches", defaultPartition);

        bool phasors = ps.getBool("Model.DirectionalGain.Phasors",
            parentConfig.phasors());

        DirectionalGainConfig config(phasors);
        config.setPartition(makePartition(itsPartitionDirectionalGain));
        itsModelConfig.setDirectionalGainConfig(config);
      }
      else {
        itsModelConfig.clearDirectionalGainConfig();
      }

      if(ps.getBool("Model.Beam.Enable", itsModelConfig.useBeam())) {
        const BeamConfig &parentConfig = itsModelConfig.getBeamConfig();

        string defaultPartition("[*]");
        if(itsModelConfig.useBeam()) {
            defaultPartition = itsPartitionBeam;
        }
        itsPartitionBeam = ps.getString("Model.Beam.Patches", defaultPartition);

        string modeString = ps.getString("Model.Beam.Mode",
          BeamConfig::asString(parentConfig.mode()));

        BeamConfig::Mode mode = BeamConfig::asMode(modeString);
        if(!BeamConfig::isDefined(mode)) {
          THROW(BBSControlException, "Invalid beam model mode specified: "
            << modeString);
        }

        bool conjugateAF = ps.getBool("Model.Beam.ConjugateAF",
            parentConfig.conjugateAF());

        string defaultPath;
        if(itsModelConfig.useBeam()) {
          defaultPath = parentConfig.getElementPath().originalName();
        } else {
          defaultPath = "$LOFARROOT/share";
        }
        string elementPath = ps.getString("Model.Beam.Element.Path",
          defaultPath);

        BeamConfig config(mode, conjugateAF, casa::Path(elementPath));
        config.setPartition(makePartition(itsPartitionBeam));
        itsModelConfig.setBeamConfig(config);
      } else {
        itsModelConfig.clearBeamConfig();
      }

      if(ps.getBool("Model.DirectionalTEC.Enable",
        itsModelConfig.useDirectionalTEC())) {

        string defaultPartition("[*]");
        if(itsModelConfig.useDirectionalTEC()) {
            defaultPartition = itsPartitionDirectionalTEC;
        }
        itsPartitionDirectionalTEC =
            ps.getString("Model.DirectionalTEC.Patches", defaultPartition);

        DDEConfig config;
        config.setPartition(makePartition(itsPartitionDirectionalTEC));
        itsModelConfig.setDirectionalTECConfig(config);
      }
      else {
        itsModelConfig.clearDirectionalTECConfig();
      }

      if(ps.getBool("Model.FaradayRotation.Enable",
        itsModelConfig.useFaradayRotation())) {

        string defaultPartition("[*]");
        if(itsModelConfig.useFaradayRotation()) {
            defaultPartition = itsPartitionFaradayRotation;
        }
        itsPartitionFaradayRotation =
            ps.getString("Model.FaradayRotation.Patches", defaultPartition);

        DDEConfig config;
        config.setPartition(makePartition(itsPartitionFaradayRotation));
        itsModelConfig.setFaradayRotationConfig(config);
      }
      else {
        itsModelConfig.clearFaradayRotationConfig();
      }

      if(ps.getBool("Model.Ionosphere.Enable",
        itsModelConfig.useIonosphere())) {

        const IonosphereConfig &parentConfig =
          itsModelConfig.getIonosphereConfig();

        string defaultPartition("[*]");
        if(itsModelConfig.useIonosphere()) {
            defaultPartition = itsPartitionIonosphere;
        }
        itsPartitionIonosphere = ps.getString("Model.Ionosphere.Patches",
            defaultPartition);

        string modelTypeString;
        if(itsModelConfig.useIonosphere()) {
          modelTypeString = ps.getString("Model.Ionosphere.Type",
            IonosphereConfig::asString(parentConfig.getModelType()));
        } else {
          modelTypeString = ps.getString("Model.Ionosphere.Type");
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
          degree = ps.getUint("Model.Ionosphere.Degree", parentConfig.degree());
        } else if(modelType == IonosphereConfig::MIM) {
          degree = ps.getUint("Model.Ionosphere.Degree");
        }

        IonosphereConfig config(modelType, degree);
        config.setPartition(makePartition(itsPartitionIonosphere));
        itsModelConfig.setIonosphereConfig(config);
      } else {
        itsModelConfig.clearIonosphereConfig();
      }

      if(ps.getBool("Model.Flagger.Enable", itsModelConfig.useFlagger())) {
        double threshold = 0.0;
        if(itsModelConfig.useFlagger()) {
          threshold = ps.getDouble("Model.Flagger.Threshold",
            itsModelConfig.getFlaggerConfig().threshold());
        } else {
          threshold = ps.getDouble("Model.Flagger.Threshold");
        }

        itsModelConfig.setFlaggerConfig(FlaggerConfig(threshold));
      } else {
        itsModelConfig.clearFlaggerConfig();
      }

      itsModelConfig.setCache(ps.getBool("Model.Cache.Enable",
        itsModelConfig.useCache()));

      itsModelConfig.setSources(ps.getStringVector("Model.Sources",
        itsModelConfig.getSources()));
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

    DDEPartition Step::makePartition(const string &specification) const
    {
        ParameterValue tmp(specification);
        ASSERT(tmp.isVector());

        DDEPartition partition;
        vector<ParameterValue> clauses = tmp.getVector();
        for(vector<ParameterValue>::const_iterator it = clauses.begin(),
            end = clauses.end(); it != end; ++it)
        {
            vector<string> clause = it->getStringVector();
            partition.append(clause.begin(), clause.end(), it->isVector());
        }

        return partition;
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
