//# VdsPartDesc.cc: Description of a visibility data set or part thereof
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

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
  }

  void VdsPartDesc::write (std::ostream& os, const std::string& prefix) const
  {
    os << prefix << "Name       = " << itsName << endl;
    if (! itsFileSys.empty()) {
      os << prefix << "FileSys    = " << itsFileSys << endl;
    }
    os << prefix << "StartTime  = "
	<< MVTime::Format(MVTime::YMD,6) << MVTime(itsStartTime/86400) << endl;
    os << prefix << "EndTime    = "
	<< MVTime::Format(MVTime::YMD,6) << MVTime(itsEndTime/86400) << endl;
    os << prefix << "StepTime   = " << itsStepTime << endl;
    if (! itsNChan.empty()) {
      os << prefix << "NChan      = " << itsNChan << endl;
      os << prefix << "StartFreqs = " << itsStartFreqs << endl;
      os << prefix << "EndFreqs   = " << itsEndFreqs << endl;
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
       >> itsNChan >> itsStartFreqs >> itsEndFreqs
       >> itsParms;
    bs.getEnd();
    return bs;
  }

}} // end namespaces
