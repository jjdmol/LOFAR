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
//# $Id$

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
      ps.replace(prefix + "Correlation.Selection",
                  itsCorrelation.selection);
      ps.replace(prefix + "Correlation.Type",
                  toString(itsCorrelation.type));
      ps.replace(prefix + "Model.UsePhasors",
                  toString(itsModelConfig.usePhasors));
      ps.replace(prefix + "Model.Sources",
                  toString(itsModelConfig.sources));
      ps.replace(prefix + "Model.Components",
		  toString(itsModelConfig.components));

      if(itsModelConfig.ionoConfig) {
        ps.replace(prefix + "Model.Ionosphere.Rank",
          toString(itsModelConfig.ionoConfig->rank));
      }

      // Cancel any inherited beam type. If we did inherit, this key will
      // be replaced again below (this is related to bug 1054).
      ps.replace(prefix + "Model.Beam.Type", "");

      if(itsModelConfig.beamConfig)
      {
        ps.replace(prefix + "Model.Beam.Type",
                  itsModelConfig.beamConfig->type());

        if(itsModelConfig.beamConfig->type() == "HamakerDipole")
        {
          HamakerDipoleConfig::ConstPointer config =
            dynamic_pointer_cast<const HamakerDipoleConfig>
              (itsModelConfig.beamConfig);
          ASSERT(config);

          ps.replace(prefix + "Model.Beam.HamakerDipole.CoeffFile",
                  config->coeffFile);
        }
        else if(itsModelConfig.beamConfig->type() == "YatawattaDipole")
        {
          YatawattaDipoleConfig::ConstPointer config =
            dynamic_pointer_cast<const YatawattaDipoleConfig>
              (itsModelConfig.beamConfig);
          ASSERT(config);

          ps.replace(prefix + "Model.Beam.YatawattaDipole.ModuleTheta",
                  config->moduleTheta);
          ps.replace(prefix + "Model.Beam.YatawattaDipole.ModulePhi",
                  config->modulePhi);
        }
      }

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
      itsModelConfig.usePhasors =
        ps.getBool("Model.UsePhasors", itsModelConfig.usePhasors);
      itsModelConfig.sources =
        ps.getStringVector("Model.Sources", itsModelConfig.sources);
      itsModelConfig.components =
        ps.getStringVector("Model.Components", itsModelConfig.components);

      if(ps.isDefined("Model.Ionosphere.Rank")) {
        IonoConfig::Pointer config(new IonoConfig());
        config->rank = ps.getUint32("Model.Ionosphere.Rank");
        itsModelConfig.ionoConfig = config;
      }

      if(ps.isDefined("Model.Beam.Type")) {
        string beamType(ps.getString("Model.Beam.Type"));

        if(beamType.empty()) {
          itsModelConfig.beamConfig.reset();
        }
        else if(beamType == "HamakerDipole") {
          HamakerDipoleConfig::ConstPointer parentConfig =
            dynamic_pointer_cast<const HamakerDipoleConfig>
              (itsModelConfig.beamConfig);

          HamakerDipoleConfig::Pointer config(new HamakerDipoleConfig());

          config->coeffFile = ps.getString("Model.Beam.HamakerDipole.CoeffFile",
            parentConfig ? parentConfig->coeffFile : string());
          if(config->coeffFile.empty()) {
            THROW(BBSControlException, "Model.Beam.HamakerDipole.CoeffFile"
              " expected but not found.");
          }

          itsModelConfig.beamConfig = config;
        }
        else if(beamType == "YatawattaDipole") {
          YatawattaDipoleConfig::ConstPointer parentConfig =
            dynamic_pointer_cast<const YatawattaDipoleConfig>
              (itsModelConfig.beamConfig);

          YatawattaDipoleConfig::Pointer config(new YatawattaDipoleConfig());

          config->moduleTheta =
            ps.getString("Model.Beam.YatawattaDipole.ModuleTheta",
              parentConfig ? parentConfig->moduleTheta : string());
          if(config->moduleTheta.empty()) {
            THROW(BBSControlException, "Model.Beam.YatawattaDipole.ModuleTheta"
              " expected but not found.");
          }

          config->modulePhi =
            ps.getString("Model.Beam.YatawattaDipole.ModulePhi", parentConfig
              ? parentConfig->modulePhi : string());
          if(config->modulePhi.empty()) {
            THROW(BBSControlException, "Model.Beam.YatawattaDipole.ModulePhi"
                " expected but not found.");
          }

          itsModelConfig.beamConfig = config;
        }
        else {
          THROW(BBSControlException, "Unknown beam model type " << beamType
            << " encountered.");
        }
      }
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
