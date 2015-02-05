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
#include <DPPP/DPRun.h>
#include <Common/ParameterSet.h>

#include <python/Converters/PycExcp.h>
#include <python/Converters/PycBasicData.h>
#include <python/Converters/PycValueHolder.h>
#include <python/Converters/PycRecord.h>
#include <python/Converters/PycArray.h>

#include <unistd.h>

using namespace casa;

namespace LOFAR {
  namespace DPPP {

    PythonStep::PythonStep (DPInput* input,
                            const ParameterSet& parset,
                            const string& prefix)
      : itsInput        (input),
        itsName         (prefix),
        itsParset       (parset.makeSubset (prefix)),
        itsPythonClass  (itsParset.getString ("python.class")),
        itsPythonModule (itsParset.getString ("python.module", itsPythonClass))
    {
      // Initialize Python interpreter.
      // Note: a second call is a no-op.
      Py_Initialize();
        // Register converters for casa types from/to python types
      casa::python::register_convert_excp();
      casa::python::register_convert_basicdata();
      casa::python::register_convert_casa_valueholder();
      casa::python::register_convert_casa_record();
      try {
        // First import main
        boost::python::object mainModule = boost::python::import
          ("__main__");
        // Import the given module
        boost::python::object dpppModule = boost::python::import
          (itsPythonModule.c_str());
        // Get the python class object from the imported module
        boost::python::object dpppAttr = dpppModule.attr
          (itsPythonClass.c_str());

        // Convert the ParameterSet to a Record (using its string values).
        Record rec;
        for (ParameterSet::const_iterator iter=itsParset.begin();
             iter!=itsParset.end(); ++iter) {
          rec.define (iter->first, iter->second.get());
        }
        // Create an instance of the python class passing the record.
        itsPyObject = dpppAttr(rec);
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
      cout<<"process"<<endl;
      itsTimer.start();
      itsBuf.referenceFilled (buf);
      if (itsNeedWeights) {
        itsInput->fetchWeights (buf, itsBuf, itsTimer);
      }
      if (itsNeedUVW) {
        itsInput->fetchUVW (buf, itsBuf, itsTimer);
      }
      if (itsNeedFullResFlags) {
        itsInput->fetchFullResFlags (buf, itsBuf, itsTimer);
      }
      cout<<"process1"<<endl;
      boost::python::object result =
        itsPyObject.attr("process")();
      cout<<"process2"<<endl;
      Record rec = boost::python::extract<Record> (result);
      cout<<"process3"<<endl;
      itsTimer.stop();
      getNextStep()->process(itsBuf);
      return false;
    }

    void PythonStep::finish()
    {
      itsPyObject.attr("finish")();
      getNextStep()->finish();
    }

    void PythonStep::updateInfo (const DPInfo& infoIn)
    {
      boost::python::object result =
        itsPyObject.attr("updateInfo")(infoIn.toRecord());
      Record rec = boost::python::extract<Record>(result);
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
      itsPyObject.attr("addToMS")(msName);
    }

    void PythonStep::show (std::ostream& os) const
    {
      boost::python::object result = itsPyObject.attr("show")();
      string str = boost::python::extract<string>(result);
      os << "PythonStep " << itsName << " class=" << itsPythonClass << endl;
      if (! str.empty()) {
        os << str;
      }
    }

    void PythonStep::showCounts (std::ostream& os) const
    {
      boost::python::object result = itsPyObject.attr("showCounts")();
      string str = boost::python::extract<string>(result);
      if (! str.empty()) {
        os << str;
      }
    }

    void PythonStep::showTimings (std::ostream& os, double duration) const
    {
      os << "  ";
      FlagCounter::showPerc1 (os, itsTimer.getElapsed(), duration);
      os << " PythonStep " << itsName << " class=" << itsPythonClass << endl;
    }


  } //# end namespace
}

// Define the function to make the PythonStep 'constructor' known.
void register_pythondppp()
{
  LOFAR::DPPP::DPRun::registerStepCtor ("pythondppp",
                                        LOFAR::DPPP::PythonStep::makeStep);
}
