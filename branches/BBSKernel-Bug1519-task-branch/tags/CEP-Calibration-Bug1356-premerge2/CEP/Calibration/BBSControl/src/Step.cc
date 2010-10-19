//#  Step.cc:
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

      ps.add(prefix + "Model.Phasors.Enabled",
        toString(itsModelConfig.usePhasors()));
      ps.add(prefix + "Model.Bandpass.Enabled",
        toString(itsModelConfig.useBandpass()));
      ps.add(prefix + "Model.IsotropicGain.Enabled",
        toString(itsModelConfig.useIsotropicGain()));
      ps.add(prefix + "Model.AnisotropicGain.Enabled",
        toString(itsModelConfig.useAnisotropicGain()));

      ps.add(prefix + "Model.Beam.Enabled", toString(itsModelConfig.useBeam()));
      if(itsModelConfig.useBeam()) {
        switch(itsModelConfig.getBeamType())
        {
          case ModelConfig::HAMAKER_DIPOLE:
          {
            HamakerDipoleConfig config;
            itsModelConfig.getBeamConfig(config);

            ps.add(prefix + "Model.Beam.Type", "HamakerDipole");
            ps.add(prefix + "Model.Beam.CoeffFile", config.getCoeffFile());
            break;
          }

          case ModelConfig::YATAWATTA_DIPOLE:
          {
            YatawattaDipoleConfig config;
            itsModelConfig.getBeamConfig(config);

            ps.add(prefix + "Model.Beam.Type", "YatawattaDipole");
            ps.add(prefix + "Model.Beam.ModuleTheta", config.getModuleTheta());
            ps.add(prefix + "Model.Beam.ModulePhi", config.getModulePhi());
            break;
          }

          default:
            break;
        }
      }

      ps.add(prefix + "Model.Ionosphere.Enabled",
        toString(itsModelConfig.useIonosphere()));
      if(itsModelConfig.useIonosphere()) {
        IonosphereConfig config;
        itsModelConfig.getIonosphereConfig(config);
        ps.add(prefix + "Model.Ionosphere.Degree",
          toString(config.getDegree()));
      }

      ps.add(prefix + "Model.Flagger.Enabled",
        toString(itsModelConfig.useFlagger()));
      if(itsModelConfig.useFlagger()) {
        FlaggerConfig config;
        itsModelConfig.getFlaggerConfig(config);
        ps.add(prefix + "Model.Flagger.Threshold",
          toString(config.getThreshold()));
      }

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
      itsModelConfig.setPhasors(ps.getBool("Model.Phasors.Enabled",
        itsModelConfig.usePhasors()));

      itsModelConfig.setBandpass(ps.getBool("Model.Bandpass.Enabled",
        itsModelConfig.useBandpass()));

      itsModelConfig.setIsotropicGain(ps.getBool("Model.IsotropicGain.Enabled",
        itsModelConfig.useIsotropicGain()));

      itsModelConfig.setAnisotropicGain
        (ps.getBool("Model.AnisotropicGain.Enabled",
          itsModelConfig.useAnisotropicGain()));

      if(ps.getBool("Model.Beam.Enabled", itsModelConfig.useBeam())) {
        ModelConfig::BeamType beamType = ModelConfig::UNKNOWN_BEAM_TYPE;

        if(itsModelConfig.useBeam()) {
          beamType = itsModelConfig.getBeamType();
        }

        if(ps.isDefined("Model.Beam.Type")) {
          string typeString(ps.getString("Model.Beam.Type"));

          if(typeString == "HamakerDipole") {
            beamType = ModelConfig::HAMAKER_DIPOLE;
          } else if(typeString == "YatawattaDipole") {
            beamType = ModelConfig::YATAWATTA_DIPOLE;
          }
        }

        switch(beamType) {
          case ModelConfig::HAMAKER_DIPOLE:
          {
            HamakerDipoleConfig parentConfig;

            if(itsModelConfig.useBeam()
              && itsModelConfig.getBeamType() == beamType) {
                itsModelConfig.getBeamConfig(parentConfig);
            }

            string coeffFile = ps.getString("Model.Beam.CoeffFile",
              parentConfig.getCoeffFile());

            if(coeffFile.empty()) {
              THROW(BBSControlException, "Key Model.Beam.CoeffFile not found.");
            }

            itsModelConfig.clearBeamConfig();
            itsModelConfig.setBeamConfig(HamakerDipoleConfig(coeffFile));
            break;
          }

          case ModelConfig::YATAWATTA_DIPOLE:
          {
            YatawattaDipoleConfig parentConfig;

            if(itsModelConfig.useBeam()
              && itsModelConfig.getBeamType() == beamType) {
                itsModelConfig.getBeamConfig(parentConfig);
            }

            string moduleTheta = ps.getString("Model.Beam.ModuleTheta",
              parentConfig.getModuleTheta());
            if(moduleTheta.empty()) {
              THROW(BBSControlException, "Key Model.Beam.ModuleTheta not"
                " found.");
            }

            string modulePhi = ps.getString("Model.Beam.ModulePhi",
              parentConfig.getModulePhi());
            if(modulePhi.empty()) {
              THROW(BBSControlException, "Key Model.Beam.ModulePhi not found.");
            }

            itsModelConfig.clearBeamConfig();
            itsModelConfig.setBeamConfig(YatawattaDipoleConfig(moduleTheta,
              modulePhi));
            break;
          }

          default:
            THROW(BBSControlException, "Key Model.Beam.Type not found or"
              " invalid.");
        }
      } else {
        itsModelConfig.clearBeamConfig();
      }

      if(ps.getBool("Model.Ionosphere.Enabled", itsModelConfig.useIonosphere()))
      {
        IonosphereConfig parentConfig;

        if(itsModelConfig.useIonosphere()) {
          itsModelConfig.getIonosphereConfig(parentConfig);
        }

        unsigned int degree = ps.getUint("Model.Ionosphere.Degree",
          parentConfig.getDegree());

        itsModelConfig.setIonosphereConfig(IonosphereConfig(degree));
      } else {
        itsModelConfig.clearIonosphereConfig();
      }

      if(ps.getBool("Model.Flagger.Enabled", itsModelConfig.useFlagger())) {
        FlaggerConfig parentConfig;

        if(itsModelConfig.useFlagger()) {
          itsModelConfig.getFlaggerConfig(parentConfig);
        }

        double threshold = ps.getDouble("Model.Flagger.Threshold",
          parentConfig.getThreshold());

        itsModelConfig.setFlaggerConfig(FlaggerConfig(threshold));
      } else {
        itsModelConfig.clearFlaggerConfig();
      }

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
