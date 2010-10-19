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
      ps.replace(prefix + "Baselines.Station1",
        toString(itsBaselines.station1));
      ps.replace(prefix + "Baselines.Station2",
        toString(itsBaselines.station2));
      ps.replace(prefix + "Correlation.Selection", itsCorrelation.selection);
      ps.replace(prefix + "Correlation.Type", toString(itsCorrelation.type));

      // Remove all Model keys from the parset. Effectively, key inheritance
      // is handled in read() and here we only output the keys that are relevant
      // for this Step.
      ps.subtractSubset(prefix + "Model.");

      ps.add(prefix + "Model.Phasors.Enable",
        toString(itsModelConfig.usePhasors()));
      ps.add(prefix + "Model.Bandpass.Enable",
        toString(itsModelConfig.useBandpass()));
      ps.add(prefix + "Model.Gain.Enable",
        toString(itsModelConfig.useGain()));
      ps.add(prefix + "Model.DirectionalGain.Enable",
        toString(itsModelConfig.useDirectionalGain()));
      ps.add(prefix + "Model.FaradayRotation.Enable",
        toString(itsModelConfig.useFaradayRotation()));

      ps.add(prefix + "Model.Beam.Enable", toString(itsModelConfig.useBeam()));
      if(itsModelConfig.useBeam()) {
        const BeamConfig &config = itsModelConfig.getBeamConfig();
        ps.add(prefix + "Model.Beam.StationConfig.Name",
          config.getConfigName());
        ps.add(prefix + "Model.Beam.StationConfig.Path",
          config.getConfigPath().originalName());
        ps.add(prefix + "Model.Beam.Element.Type",
          config.getElementTypeAsString());
        ps.add(prefix + "Model.Beam.Element.Path",
          config.getElementPath().originalName());
      }

      ps.add(prefix + "Model.Ionosphere.Enable",
        toString(itsModelConfig.useIonosphere()));
      if(itsModelConfig.useIonosphere()) {
        const IonosphereConfig &config = itsModelConfig.getIonosphereConfig();
        ps.add(prefix + "Model.Ionosphere.Degree",
          toString(config.getDegree()));
      }

      ps.add(prefix + "Model.Flagger.Enable",
        toString(itsModelConfig.useFlagger()));
      if(itsModelConfig.useFlagger()) {
        const FlaggerConfig &config = itsModelConfig.getFlaggerConfig();
        ps.add(prefix + "Model.Flagger.Threshold",
          toString(config.getThreshold()));
      }

      ps.add(prefix + "Model.ExperimentalCaching.Enable",
        toString(itsModelConfig.useExperimentalCaching()));

      ps.add(prefix + "Model.Sources", toString(itsModelConfig.getSources()));

      LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
    }


    void Step::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Get the baseline selection for this step.
      itsBaselines.station1 =
        ps.getStringVector("Baselines.Station1", itsBaselines.station1);
      itsBaselines.station2 =
        ps.getStringVector("Baselines.Station2", itsBaselines.station2);

      // Get the correlation selection (ALL, AUTO, or CROSS), and type
      // (e.g., ["XX", "XY", "YX", "YY"]).
      itsCorrelation.selection =
        ps.getString("Correlation.Selection", itsCorrelation.selection);
      itsCorrelation.type =
        ps.getStringVector("Correlation.Type", itsCorrelation.type);

      // Get the model configuration.
      itsModelConfig.setPhasors(ps.getBool("Model.Phasors.Enable",
        itsModelConfig.usePhasors()));

      itsModelConfig.setBandpass(ps.getBool("Model.Bandpass.Enable",
        itsModelConfig.useBandpass()));

      itsModelConfig.setGain(ps.getBool("Model.Gain.Enable",
        itsModelConfig.useGain()));

      itsModelConfig.setDirectionalGain
        (ps.getBool("Model.DirectionalGain.Enable",
          itsModelConfig.useDirectionalGain()));

      itsModelConfig.setFaradayRotation
        (ps.getBool("Model.FaradayRotation.Enable",
          itsModelConfig.useFaradayRotation()));

      if(ps.getBool("Model.Beam.Enable", itsModelConfig.useBeam())) {
        BeamConfig parentConfig = itsModelConfig.getBeamConfig();

        string elementTypeString = ps.getString("Model.Beam.Element.Type",
          parentConfig.getElementTypeAsString());
        BeamConfig::ElementType elementType =
          BeamConfig::getElementTypeFromString(elementTypeString);

        if(elementType == BeamConfig::UNKNOWN) {
          THROW(BBSControlException, "Key Model.Beam.Element.Type not found or"
            " invalid.");
        }

        string defaultPath;
        if(itsModelConfig.useBeam()
          && parentConfig.getElementType() == elementType) {
          defaultPath = parentConfig.getElementPath().originalName();
        } else if(elementType == BeamConfig::HAMAKER_LBA
          || elementType == BeamConfig::HAMAKER_HBA) {
          defaultPath = "$LOFARROOT/share";
        } else {
          defaultPath = "$LOFARROOT/lib";
        }

        string elementPath = ps.getString("Model.Beam.Element.Path",
          defaultPath);

        string configName, configPath;
        if(itsModelConfig.useBeam()) {
          configName = ps.getString("Model.Beam.StationConfig.Name",
            parentConfig.getConfigName());
          configPath = ps.getString("Model.Beam.StationConfig.Path",
            parentConfig.getConfigPath().originalName());
        } else {
          configName = ps.getString("Model.Beam.StationConfig.Name");
          configPath = ps.getString("Model.Beam.StationConfig.Path");
        }

        itsModelConfig.setBeamConfig(BeamConfig(configName,
          casa::Path(configPath), elementType, casa::Path(elementPath)));
      } else {
        itsModelConfig.clearBeamConfig();
      }

      if(ps.getBool("Model.Ionosphere.Enable", itsModelConfig.useIonosphere()))
      {
        unsigned int degree = 0;
        if(itsModelConfig.useIonosphere()) {
          degree = ps.getUint("Model.Ionosphere.Degree",
            itsModelConfig.getIonosphereConfig().getDegree());
        } else {
          degree = ps.getUint("Model.Ionosphere.Degree");
        }

        itsModelConfig.setIonosphereConfig(IonosphereConfig(degree));
      } else {
        itsModelConfig.clearIonosphereConfig();
      }

      if(ps.getBool("Model.Flagger.Enable", itsModelConfig.useFlagger())) {
        double threshold = 0.0;
        if(itsModelConfig.useFlagger()) {
          threshold = ps.getDouble("Model.Flagger.Threshold",
            itsModelConfig.getFlaggerConfig().getThreshold());
        } else {
          threshold = ps.getDouble("Model.Flagger.Threshold");
        }

        itsModelConfig.setFlaggerConfig(FlaggerConfig(threshold));
      } else {
        itsModelConfig.clearFlaggerConfig();
      }

      itsModelConfig.setExperimentalCaching(ps.getBool("Model.ExperimentalCaching.Enable",
        itsModelConfig.useExperimentalCaching()));

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
    	  << endl << indent << itsBaselines
    	  << endl << indent << itsCorrelation
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
