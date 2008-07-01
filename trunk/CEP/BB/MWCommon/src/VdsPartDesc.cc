//# VdsPartDesc.cc: Description of a visibility data set or part thereof
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <MWCommon/VdsPartDesc.h>
#include <Common/StreamUtil.h>
#include <ostream>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;

namespace LOFAR { namespace CEP {

  VdsPartDesc::VdsPartDesc (const ParameterSet& parset)
  {
    itsName       = parset.getString ("Name");
    itsFileSys    = parset.getString ("FileSys", "");
    itsStartTime  = parset.getDouble ("StartTime");
    itsEndTime    = parset.getDouble ("EndTime");
    itsStepTime   = parset.getDouble ("StepTime");
    itsNChan      = parset.getInt32Vector ("NChan", vector<int32>());
    itsStartFreqs = parset.getDoubleVector ("StartFreqs", vector<double>());
    itsEndFreqs   = parset.getDoubleVector ("EndFreqs", vector<double>());
  }

  void VdsPartDesc::write (std::ostream& os, const std::string& prefix) const
  {
    os << prefix << "Name       = " << itsName << endl;
    if (! itsFileSys.empty()) {
      os << prefix << "FileSys    = " << itsFileSys << endl;
    }
    os << prefix << "StartTime  = " << itsStartTime << endl;
    os << prefix << "EndTime    = " << itsEndTime << endl;
    os << prefix << "StepTime   = " << itsStepTime << endl;
    if (! itsNChan.empty()) {
      os << prefix << "NChan      = " << itsNChan << endl;
      os << prefix << "StartFreqs = " << itsStartFreqs << endl;
      os << prefix << "EndFreqs   = " << itsEndFreqs << endl;
    }
  }

  void VdsPartDesc::setName (const std::string& name,
                             const std::string& fileSys)
  {
    itsName    = name;
    itsFileSys = fileSys;
  }

    void VdsPartDesc::setTimes (double startTime, double endTime, double stepTime)
  {
    itsStartTime = startTime;
    itsEndTime   = endTime;
    itsStepTime  = stepTime;
  }

  void VdsPartDesc::addBand (int nchan, double startFreq, double endFreq)
  {
    itsNChan.push_back (nchan);
    double step = (endFreq-startFreq)/nchan;
    for (int i=0; i<nchan; ++i) {
      itsStartFreqs.push_back (startFreq);
      startFreq += step;
      itsEndFreqs.push_back (startFreq);
    }
  }

}} // end namespaces
