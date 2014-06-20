//#  SolveStep.cc:
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

#include <BBSControl/SolveStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/StreamUtil.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

#include <casa/Quanta/MVAngle.h>

#include <limits>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;


    //##--------   P u b l i c   m e t h o d s   --------##//

    SolveStep::SolveStep(const Step* parent) :
      SingleStep(parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    SolveStep::SolveStep(const string& name,
			       const ParameterSet& parset,
			       const Step* parent)
        :   SingleStep(name, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Get the relevant parameters from the Parameter Set \a parset.
      read(parset.makeSubset("Step." + name + "."));
    }


    SolveStep::~SolveStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    CommandResult SolveStep::accept(CommandVisitor &visitor) const
    {
      return visitor.visit(*this);
    }


    void SolveStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::print(os);
      Indent id;
      os << endl << indent << "Solve: ";
      {
        Indent id;
        os << endl << indent << "Mode: " << itsMode
          << endl << indent << "Algorithm: " << itsAlgorithm
          << endl << indent << "L1 epsilon values: " << itsEpsilonL1
          << endl << indent << "Solvable parameters: " << itsParms
          << endl << indent << "Excluded parameters: " << itsExclParms
          << endl << indent << "Calibration groups: " << itsCalibrationGroups
          << endl << indent << itsCellSize
          << endl << indent << "Cell chunk size: " << itsCellChunkSize
          << endl << indent << "Propagate solutions: " << boolalpha
          << itsPropagateFlag << noboolalpha;

        os << endl << indent << "Log:";
        {
          Indent id;
          os << endl << indent << "Name: " << itsLogName
            << endl << indent << "Level: " << itsLogLevel;
        }

        os << endl << indent << "Flag on UV interval: " << boolalpha
          << itsUVFlag << noboolalpha;
        if(itsUVFlag) {
          Indent id;
          os << endl << indent << "UV interval: [" << itsUVRange.first
            << "," << itsUVRange.second << "] (wavelenghts)";
        }

        os << endl << indent << "Outlier rejection: " << boolalpha
          << itsRejectFlag << noboolalpha;
        if(itsRejectFlag) {
          Indent id;
          os << endl << indent << "RMS threshold: "
            << itsRMSThreshold;
        }

        os << endl << indent << "Resample observed data: " << boolalpha
          << itsResampleFlag << noboolalpha;
        if(itsResampleFlag) {
          Indent id;
          os << endl << indent << "Resample cell size: ";
          {
            Indent id;
            os << endl << indent << "Frequency: " << itsResampleCellSize.freq
              << " (channels)" << endl << indent << "Time: "
              << itsResampleCellSize.time << " (timeslots)";
          }
          os << endl << indent << "Density threshold: " << itsDensityThreshold;
        }

        os << endl << indent << "Phase shift observed data: " << boolalpha
          << itsShiftFlag << noboolalpha;
        if(itsShiftFlag) {
          Indent id;
          os << endl << indent << "Direction: " << itsDirectionASCII;
        }

        os << endl << indent << "Solver options:";
        {
          Indent id;
          os << endl << indent << "Max nr. of iterations: " << itsMaxIter
            << endl << indent << "Epsilon value: " << itsEpsValue
            << endl << indent << "Epsilon derivative: " << itsEpsDerivative
            << endl << indent << "Colinearity factor: " << itsColFactor
            << endl << indent << "LM factor: " << itsLMFactor
            << boolalpha
            << endl << indent << "Balanced equations: " << itsBalancedEq
            << endl << indent << "Use SVD: " << itsUseSVD
            << noboolalpha;
        }
      }
    }


    const string& SolveStep::type() const
    {
      static const string theType("Solve");
      return theType;
    }


    const string& SolveStep::operation() const
    {
      static string theOperation("Solve");
      return theOperation;
    }

    //##--------   P r i v a t e   m e t h o d s   --------##//

    void SolveStep::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::write(ps);
      const string prefix = "Step." + name() + ".Solve.";
      ps.replace(prefix + "Mode", itsMode);
      ps.replace(prefix + "Algorithm", itsAlgorithm);
      ps.replace(prefix + "EpsilonL1", toString(itsEpsilonL1));
      ps.replace(prefix + "Parms", toString(itsParms));
      ps.replace(prefix + "ExclParms", toString(itsExclParms));
      ps.replace(prefix + "CalibrationGroups", toString(itsCalibrationGroups));
      ps.replace(prefix + "CellSize.Freq", toString(itsCellSize.freq));
      ps.replace(prefix + "CellSize.Time", toString(itsCellSize.time));
      ps.replace(prefix + "CellChunkSize", toString(itsCellChunkSize));
      ps.replace(prefix + "PropagateSolutions", toString(itsPropagateFlag));
      ps.replace(prefix + "Log.Name", itsLogName);
      ps.replace(prefix + "Log.Level", itsLogLevel);

      if(itsUVFlag)
      {
        ps.replace(prefix + "UVRange", "[" + toString(itsUVRange.first)
          + "," + toString(itsUVRange.second) + "]");
      }

      ps.replace(prefix + "OutlierRejection.Enable", toString(itsRejectFlag));
      if(itsRejectFlag) {
        ps.replace(prefix + "OutlierRejection.Threshold",
          toString(itsRMSThreshold));
      }

      ps.replace(prefix + "PhaseShift.Enable", toString(itsShiftFlag));
      if(itsShiftFlag) {
        ps.replace(prefix + "PhaseShift.Direction",
          toString(itsDirectionASCII));
      }

      ps.replace(prefix + "Resample.Enable", toString(itsResampleFlag));
      if(itsResampleFlag) {
        ps.replace(prefix + "Resample.CellSize.Freq",
          toString(itsResampleCellSize.freq));
        ps.replace(prefix + "Resample.CellSize.Time",
          toString(itsResampleCellSize.time));
        ps.replace(prefix + "Resample.DensityThreshold",
          toString(itsDensityThreshold));
      }

      ps.replace(prefix + "Options.MaxIter", toString(itsMaxIter));
      ps.replace(prefix + "Options.EpsValue", toString(itsEpsValue));
      ps.replace(prefix + "Options.EpsDerivative", toString(itsEpsDerivative));
      ps.replace(prefix + "Options.ColFactor", toString(itsColFactor));
      ps.replace(prefix + "Options.LMFactor", toString(itsLMFactor));
      ps.replace(prefix + "Options.BalancedEqs", toString(itsBalancedEq));
      ps.replace(prefix + "Options.UseSVD", toString(itsUseSVD));

      LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
    }


    void SolveStep::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      SingleStep::read(ps);
      ParameterSet pss(ps.makeSubset("Solve."));

      itsMode = pss.getString("Mode", "COMPLEX");
      itsAlgorithm = pss.getString("Algorithm", "L2");
      double defEpsilon[3] = {1e-4, 1e-5, 1e-6};
      itsEpsilonL1 = pss.getDoubleVector("EpsilonL1",
        vector<double>(defEpsilon, defEpsilon + 3));

      itsParms = pss.getStringVector("Parms");
      if (itsParms.empty()) {
        THROW(BBSControlException, "Key Solve.Parms contains no solvable "
              "parameters for step \"" << name() << "\"");
      }
      itsExclParms = pss.getStringVector("ExclParms", vector<string>());
      itsCalibrationGroups = pss.getUint32Vector("CalibrationGroups",
        vector<uint32>());
      itsCellSize.freq = pss.getUint32("CellSize.Freq");
      itsCellSize.time = pss.getUint32("CellSize.Time");
      itsCellChunkSize = pss.getUint32("CellChunkSize");
      itsPropagateFlag = pss.getBool("PropagateSolutions", false);
      itsLogName = pss.getString("Log.Name", "solver_log");
      itsLogLevel = pss.getString("Log.Level", "NONE");

      setUVRange(pss);

      itsRejectFlag = pss.getBool("OutlierRejection.Enable", false);
      if(itsRejectFlag) {
        // Default RMS thresholds taken from the AIPS CALIB help text.
        double defThreshold[10] = {7.0, 5.0, 4.0, 3.5, 3.0, 2.8, 2.6, 2.4, 2.2,
          2.5};
        itsRMSThreshold = pss.getDoubleVector("OutlierRejection.Threshold",
          vector<double>(defThreshold, defThreshold + 10));
      }

      itsDirection = casa::MDirection();
      itsDirectionASCII = vector<string>();
      itsShiftFlag = pss.getBool("PhaseShift.Enable", false);
      if(itsShiftFlag) {
        setDirection(pss);
      }

      itsResampleCellSize = CellSize(1, 1);
      itsResampleFlag = pss.getBool("Resample.Enable", false);
      if(itsResampleFlag) {
        setResampleCellSize(pss);
        itsDensityThreshold = pss.getDouble("Resample.DensityThreshold", 1.0);
      }

      itsMaxIter = pss.getUint32("Options.MaxIter", 0);
      itsEpsValue = pss.getDouble("Options.EpsValue", 1e-8);
      itsEpsDerivative = pss.getDouble("Options.EpsDerivative", 1e-8);
      itsColFactor = pss.getDouble("Options.ColFactor", 1e-6);
      itsLMFactor = pss.getDouble("Options.LMFactor", 1e-3);
      itsBalancedEq = pss.getBool("Options.BalancedEqs", false);
      itsUseSVD = pss.getBool("Options.UseSVD", false);
    }

    void SolveStep::setUVRange(const ParameterSet& ps)
    {
      itsUVFlag = ps.isDefined("UVRange");

      if(!itsUVFlag)
      {
        itsUVRange = make_pair(0.0, std::numeric_limits<double>::max());
        return;
      }

      vector<double> range = ps.getDoubleVector("UVRange");
      if(range.size() > 2)
      {
        THROW(BBSControlException, "UVRange should be given as either [min]"
          " or [min, max] (in wavelenghts)");
      }

      itsUVRange.first = range.size() > 0 ? range[0] : 0.0;
      itsUVRange.second = range.size() > 1 ? range[1]
        : std::numeric_limits<double>::max();

      if(itsUVRange.first > itsUVRange.second)
      {
        THROW(BBSControlException, "Invalid UVRange: [" << itsUVRange.first
            << "," << itsUVRange.second << "]");
      }
    }

    void SolveStep::setResampleCellSize(const ParameterSet& ps)
    {
      itsResampleCellSize.freq = ps.getUint32("Resample.CellSize.Freq");
      itsResampleCellSize.time = ps.getUint32("Resample.CellSize.Time");

      if(itsResampleCellSize.freq == 0 || itsResampleCellSize.time == 0)
      {
        THROW(BBSControlException, "Invalid resample cell size specified for "
          "step: " << name());
      }
    }

    void SolveStep::setDirection(const ParameterSet& ps)
    {
      // Parse PhaseShift.Direction value and convert to casa::MDirection.
      itsDirectionASCII = ps.getStringVector("PhaseShift.Direction");
      if(itsDirectionASCII.size() != 3)
      {
        THROW(BBSControlException, "Parse error in key PhaseShift of step: "
          << name());
      }

      casa::Bool status = true;
      casa::MDirection::Types refType;
      status = status && casa::MDirection::getType(refType,
        itsDirectionASCII[0]);

      casa::Quantity ra, dec;
      status = status && casa::MVAngle::read(ra, itsDirectionASCII[1]);
      status = status && casa::MVAngle::read(dec, itsDirectionASCII[2]);

      if(!status)
      {
        THROW(BBSControlException, "Parse error in key PhaseShift of step: "
          << name());
      }

      itsDirection = casa::MDirection(ra, dec, refType);
    }


  } // namespace BBS

} // namespace LOFAR
