//# PythonStep.cc: A DPStep executed in some python module
//# Copyright (C) 2015
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
//# $Id: ApplyCal.cc 21598 2012-07-16 08:07:34Z diepen $
//#
//# @author Tammo Jan Dijkema

#include <lofar_config.h>
#include <PythonDPPP/PythonStep.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <Common/ParameterSet.h>

#include <python/Converters/PycExcp.h>
#include <python/Converters/PycBasicData.h>
#include <python/Converters/PycValueHolder.h>
#include <python/Converters/PycRecord.h>
#include <python/Converters/PycArray.h>

using namespace casa;
using namespace LOFAR::BBS;

namespace LOFAR {
  namespace DPPP {

    PythonStep::PythonStep (DPInput* input,
                            const ParameterSet& parset,
                            const string& prefix)
      : itsInput       (input),
        itsName        (prefix),
        itsParset      (parset.makeSubset (prefix)),
        itsPythonName  (itsParset.getString ("python.module"))
    {
      // Initialize Python interpreter
      Py_Initialize();
        // Register converters for casa types from/to python types
      casa::pyrap::register_convert_excp();
      casa::pyrap::register_convert_basicdata();
      casa::pyrap::register_convert_casa_valueholder();
      casa::pyrap::register_convert_casa_record();
      try {
        // First import main
        boost::python::object main_module = boost::python::import
          ("__main__");
        // Import the given module
        boost::python::object embedded_module = boost::python::import
          (parameters.getString(itsPythonName + ".module").c_str());
        // Get the python class object from the imported module
        boost::python::object pyAttr = embedded_module.attr
          (parameters.getString(itsPythonName + ".class").c_str());

        // Import the lofar.parameterset module
        boost::python::object lofar_parameterset_module = boost::python::import
          ("lofar.parameterset");
        boost::python::object pyparameterset = lofar_parameterset_module.attr
          ("parameterset")();
        ParameterSet ps = boost::python::extract<ParameterSet>(pyparameterset);
        
        ps.adoptCollection(parameters);
    
        // Create an instance of the python class
        itsPyObject = pyAttr(pyparameterset); 
      } catch (boost::python::error_already_set const &) {
        // handle the exception in some way
        PyErr_Print();
      }
    }

    PythonStep::~PythonStep()
    {
    }

    DPStep::ShPtr PythonStep::makeStep (DPInput* input,
                                        const ParameterSet& pset,
                                        const std::string& prefix)
    {
      return DPStep::ShPtr(new PythonStep(input, pset, prefix));
    }

    bool PythonStep::process (const DPBuffer& buf)
    {
      itsTimer.start();
      itsBuf.referenceFilled (buf);
      if (itsNeedWeights) {
        itsInput.fetchWeights (buf, itsBuf, itsTimer);
      }
      if (itsNeedUVW) {
        itsInput.fetchUVW (buf, itsBuf, itsTimer);
      }
      if (itsNeedFullResFlags) {
        itsInput.fetchFullResFlags (buf, itsBuf, itsTimer);
      }
      itsTimer.stop();
      getNextStep()->process(itsBuf);
      return false;
    }

    void PythonStep::finish()
    {
      itsPyObject.attr("finish")();
      getNextStep.finish();
    }

    void PythonStep::updateInfo (const DPInfo& infoIn)
    {
      Record rec = itsPyObject.attr("updateInfo")(infoIn.toRecord());
      info() = infoIn;
      // Merge possible result back in DPInfo object.
      info().fromRecord (rec);
      // See which data parts are needed (to avoid sending too much to python).
      if (rec.isDefined("NeedWeights")) {
        rec.get ("NeedWeights", itsNeedWeights);
      }
      if (rec.isDefined("NeedUVW")) {
        rec.get ("NeedUVW", itsNeedUVW);
      }
      if (rec.isDefined("NeedFullResFlags")) {
        rec.get ("NeedFullresFlags", itsNeedFullResFlags);
      }
    }

    void PythonStep::addToMS (const string& msName)
    {
      itsPyObject.attr("addToMS")();
    }

    void PythonStep::show (std::ostream& os) const
    {
      string result = itsPyObject.attr("show")();
      os << result;
    }

    void PythonStep::showCounts (std::ostream& os) const
    {
      string result = itsPyObject.attr("showCounts")();
      os << result;
    }

    void PythonStep::showTimings (std::ostream&, double duration) const
    {
      os << "  ";
      FlagCounter::showPerc1 (os, itsTimer.getElapsed(), duration);
      os << " PythonStep " << itsPythonName << endl;
    }


  } //# end namespace
}

// Define the function to make the PythonStep 'constructor' known.
void register_pythondppp()
{
  LOFAR::DPPP::DPRun::registerStepCtor ("pythondppp",
                                        LOFAR::DPPP::PythonStep::makeStep);
}
