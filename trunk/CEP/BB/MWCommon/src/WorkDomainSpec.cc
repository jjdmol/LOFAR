//# WorkDomainSpec.cc: Define the specifications of the work domain
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/WorkDomainSpec.h>
#include <MWCommon/MWError.h>
#include <Blob/BlobArray.h>

using namespace std;


namespace LOFAR { namespace CEP {

  void WorkDomainSpec::setAntennas (const vector<int>& antNrs)
  {
    itsAntNrs = antNrs;
  }

  void WorkDomainSpec::setAntennaNames (const vector<string>& antNames)
  {
    itsAntNames = antNames;
  }

  void WorkDomainSpec::setCorr (const vector<bool>& corr)
  {
    itsCorr = corr;
  }

  LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
				  const WorkDomainSpec& wds)
  {
    bs.putStart ("WDS", 1);
    bs << wds.itsInColumn
       << wds.itsAntNrs
       << wds.itsAntNames
       << wds.itsAutoCorr
       << wds.itsCorr
       << wds.itsShape
       << wds.itsFreqInt
       << wds.itsTimeInt;
    return bs;
  }

  LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
				  WorkDomainSpec& wds)
  {
    int vers = bs.getStart ("WDS");
    ASSERT (vers == 1);
    bs >> wds.itsInColumn
       >> wds.itsAntNrs
       >> wds.itsAntNames
       >> wds.itsAutoCorr
       >> wds.itsCorr
       >> wds.itsShape
       >> wds.itsFreqInt
       >> wds.itsTimeInt;
    return bs;
  }


}} // end namespaces
