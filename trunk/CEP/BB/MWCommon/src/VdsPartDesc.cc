//# VdsPartDesc.cc: Description of a visibility data set or part thereof
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/VdsPartDesc.h>
#include <MWCommon/ParameterHandler.h>
#include <Blob/BlobArray.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <casa/Quanta/MVTime.h>
#include <ostream>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;
using namespace casa;
using LOFAR::ACC::APS::ParameterSet;

namespace LOFAR { namespace CEP {

  VdsPartDesc::VdsPartDesc (const ParameterSet& parset)
  {
    itsName         = parset.getString ("Name");
    itsFileSys      = parset.getString ("FileSys", "");
    itsStepTime     = parset.getDouble ("StepTime");
    itsNChan        = parset.getInt32Vector ("NChan", vector<int32>());
    itsStartFreqs   = parset.getDoubleVector ("StartFreqs", vector<double>());
    itsEndFreqs     = parset.getDoubleVector ("EndFreqs", vector<double>());
    itsParms        = parset.makeSubset ("Extra.");
    string timeStr;
    Quantity q;
    timeStr = parset.getString ("StartTime");
    ASSERT (MVTime::read (q, timeStr, true));
    itsStartTime = q.getValue ("s");
    timeStr = parset.getString ("EndTime");
    ASSERT (MVTime::read (q, timeStr, true));
    itsEndTime = q.getValue ("s");
    itsStartTimes = parset.getDoubleVector ("StartTimesDiff", vector<double>());
    itsEndTimes   = parset.getDoubleVector ("EndTimesDiff",   vector<double>());
    ASSERT (itsStartTimes.size() == itsEndTimes.size());
    double diff = itsStartTime;
    for (uint i=0; i<itsStartTimes.size(); ++i) {
      itsStartTimes[i] += diff;
      diff += itsStepTime;
      itsEndTimes[i]   += diff;
    }
  }

  void VdsPartDesc::write (std::ostream& os, const std::string& prefix) const
  {
    os << prefix << "Name       = " << itsName << endl;
    if (! itsFileSys.empty()) {
      os << prefix << "FileSys    = " << itsFileSys << endl;
    }
    os << prefix << "StartTime  = "
	<< MVTime::Format(MVTime::YMD,9) << MVTime(itsStartTime/86400) << endl;
    os << prefix << "EndTime    = "
	<< MVTime::Format(MVTime::YMD,9) << MVTime(itsEndTime/86400) << endl;
    os << prefix << "StepTime   = " << itsStepTime << endl;
    if (! itsStartTimes.empty()) {
      os << prefix << "StartTimesDiff=[";
      streamsize oldPrec = os.precision (5);
      double diff = itsStartTime;
      for (uint i=0; i<itsStartTimes.size(); ++i) {
        if (i!=0) os << ',';
        os << itsStartTimes[i] - diff;
        diff += itsStepTime;
      }
      os << ']' << endl;
      os.precision (oldPrec);
    }
    if (! itsEndTimes.empty()) {
      os << prefix << "EndTimesDiff=[";
      streamsize oldPrec = os.precision (5);
      double diff = itsStartTime;
      for (uint i=0; i<itsEndTimes.size(); ++i) {
        if (i!=0) os << ',';
        diff += itsStepTime;
        os << itsEndTimes[i] - diff;
      }
      os << ']' << endl;
      os.precision (oldPrec);
    }
    if (! itsNChan.empty()) {
      os << prefix << "NChan      = " << itsNChan << endl;
      streamsize oldPrec = os.precision (12);
      os << prefix << "StartFreqs = " << itsStartFreqs << endl;
      os << prefix << "EndFreqs   = " << itsEndFreqs << endl;
      os.precision (oldPrec);
    }
    // Prepend the extra parameters with Extra..
    ParameterSet parms;
    parms.adoptCollection (itsParms, prefix+"Extra.");
    parms.writeStream (os);
  }

  void VdsPartDesc::setName (const std::string& name,
                             const std::string& fileSys)
  {
    itsName    = name;
    itsFileSys = fileSys;
  }

  void VdsPartDesc::changeBaseName (const string& newBaseName)
  {
    string::size_type pos = itsName.rfind ('/');
    if (pos == string::npos) {
      itsName = newBaseName;
    } else {
      itsName = itsName.substr (0, pos+1) + newBaseName;
    }
  }

  void VdsPartDesc::setTimes (double startTime, double endTime, double stepTime,
                              const vector<double>& startTimes,
                              const vector<double>& endTimes)
  {
    ASSERT (itsStartTimes.size() == itsEndTimes.size());
    itsStartTime  = startTime;
    itsEndTime    = endTime;
    itsStepTime   = stepTime;
    itsStartTimes = startTimes;
    itsEndTimes   = endTimes;
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

  void VdsPartDesc::addBand (int nchan, const vector<double>& startFreq,
			     const vector<double>& endFreq)
  {
    ASSERT (startFreq.size() == endFreq.size());
    ASSERT (int(startFreq.size())==nchan || startFreq.size() == 1);
    itsNChan.push_back (nchan);
    for (uint i=0; i<startFreq.size(); ++i) {
      itsStartFreqs.push_back (startFreq[i]);
      itsEndFreqs.push_back   (endFreq[i]);
    }
  }

  BlobOStream& VdsPartDesc::toBlob (BlobOStream& bs) const
  {
    bs.putStart ("VdsPartDesc", 1);
    bs << itsName << itsFileSys
       << itsStartTime << itsEndTime << itsStepTime
       << itsStartTimes << itsEndTimes
       << itsNChan << itsStartFreqs << itsEndFreqs
       << itsParms;
    bs.putEnd();
    return bs;
  }

  BlobIStream& VdsPartDesc::fromBlob (BlobIStream& bs)
  {
    bs.getStart ("VdsPartDesc");
    bs >> itsName >> itsFileSys
       >> itsStartTime >> itsEndTime >> itsStepTime
       >> itsStartTimes >> itsEndTimes
       >> itsNChan >> itsStartFreqs >> itsEndFreqs
       >> itsParms;
    bs.getEnd();
    return bs;
  }

}} // end namespaces
