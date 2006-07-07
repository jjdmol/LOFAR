//  DH_Solution.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <BBS3/DH_Solution.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <sstream>
#include <TransportPostgres/TH_DB.h>

namespace LOFAR
{

#define PRECISION 20

const unsigned int MaxNumberOfParam = 32;  // These need NOT be the same as in DH_WorkOrder
const unsigned int MaxParamNameLength = 16;

DH_Solution::DH_Solution (const string& name, bool writeIndivParms,
			  const string& parmTableName)
  : DH_DB             (name, "DH_Solution", 1),
    itsWOID           (0),
    itsIteration      (0),
    itsFit            (0),
    //    itsRank         (0),
    itsMu             (0),
    itsStdDev         (0),
    itsChi            (0),
    itsStartFreq      (0),
    itsEndFreq        (0),
    itsStartTime      (0),
    itsEndTime        (0),
    itsHasConverged   (0),
    itsWriteIndivParms(writeIndivParms),
    itsParmTableName  (parmTableName)
{
  setExtraBlob("Extra", 1);
}

DH_Solution::DH_Solution(const DH_Solution& that)
  : DH_DB             (that),
    itsWOID           (0),
    itsIteration      (0),
    itsFit            (0),
    //    itsRank         (0),
    itsMu             (0),
    itsStdDev         (0),
    itsChi            (0),
    itsStartFreq      (0),
    itsEndFreq        (0),
    itsStartTime      (0),
    itsEndTime        (0),
    itsHasConverged   (0),
    itsWriteIndivParms(that.itsWriteIndivParms),
    itsParmTableName  (that.itsParmTableName)
{
  setExtraBlob("Extra", 1);
}

DH_Solution::~DH_Solution()
{
}

DataHolder* DH_Solution::clone() const
{
  return new DH_Solution(*this);
}

void DH_Solution::init()
{
  // Add the fields to the data definition.
  addField ("WOID", BlobField<int>(1));
  addField ("Iteration", BlobField<int>(1));
  addField ("Fit", BlobField<double>(1));
  //  addField ("Rank", BlobField<int>(1));
  addField ("Mu", BlobField<double>(1));
  addField ("StdDev", BlobField<double>(1));
  addField ("Chi", BlobField<double>(1));
  addField ("StartFreq", BlobField<double>(1));
  addField ("EndFreq", BlobField<double>(1));
  addField ("StartTime", BlobField<double>(1));
  addField ("EndTime", BlobField<double>(1));
  addField ("HasConverged", BlobField<unsigned int>(1));

  // Create the data blob (which calls fillPointers).
  createDataBlock();

  *itsWOID = -1;
  *itsIteration = -1;
  *itsFit = 0;
  //  *itsRank = 0;
  *itsMu = 0;
  *itsStdDev =0;
  *itsChi = 0;
  *itsStartFreq = 0;
  *itsEndFreq = 0;
  *itsStartTime = 0;
  *itsEndTime = 0;
  *itsHasConverged = 0;
}

void DH_Solution::fillDataPointers()
{
 // Fill in the pointers.
  itsWOID = getData<int> ("WOID");
  itsIteration = getData<int> ("Iteration");
  itsFit = getData<double> ("Fit");
  //  itsRank = getData<int> ("Rank");
  itsMu = getData<double> ("Mu");
  itsStdDev = getData<double> ("StdDev");
  itsChi = getData<double> ("Chi");
  itsStartFreq = getData<double> ("StartFreq");
  itsEndFreq = getData<double> ("EndFreq");
  itsStartTime = getData<double> ("StartTime");
  itsEndTime = getData<double> ("EndTime");
  itsHasConverged = getData<unsigned int> ("HasConverged");
}

Quality DH_Solution::getQuality() const
{
  Quality qual;
  qual.itsFit = *itsFit;
  //  qual.itsRank = *itsRank;
  qual.itsMu = *itsMu;
  qual.itsStddev = *itsStdDev;
  qual.itsChi = *itsChi;
  return qual;
}

void DH_Solution::setQuality(const Quality& quality)
{
  *itsFit = quality.itsFit;
  //  *itsRank = quality.itsRank;
  *itsMu = quality.itsMu;
  *itsStdDev = quality.itsStddev;
  *itsChi = quality.itsChi;
}

bool DH_Solution::getSolution(vector<ParmData>& pData)
{
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob(found, version);
  if (!found) {
    return false;
  }
  else
  {
    // Get parameter data
    pData.clear();
    int size;
    bis >> size;
    pData.resize(size);
    for (int i=0; i<size; i++)
    { 
      bis >> pData[i];
    }
    bis.getEnd();
    return true;
  }  

}

void DH_Solution::setSolution(const vector<ParmData>& pData)
{
  BlobOStream& bos = createExtraBlob();
  // Put vector length into extra blob
  vector<ParmData>::const_iterator iter;
  int nParms = pData.size();
  bos << nParms;
  for (iter = pData.begin(); iter != pData.end(); iter++)
  {
    bos << *iter;
  }
}

void DH_Solution::setDomain(double fStart, double fEnd, double tStart, double tEnd)
{
  *itsStartFreq = fStart;
  *itsEndFreq = fEnd;
  *itsStartTime = tStart;
  *itsEndTime = tEnd;
}


void DH_Solution::clearData()
{
  setWorkOrderID(-1);
  setIteration(-1);
  Quality q;
  setQuality(q);
  clearExtraBlob();
  setDomain(0, 0, 0, 0);
}

void DH_Solution::dump() const
{
  vector<ParmData> pData;
  const_cast<DH_Solution*>(this)->getSolution(pData);
  
  cout << "Workorder id = " << getWorkOrderID() << endl;
  cout << "Iteration = " << getIteration() << endl;
  cout << "Parm data : " << endl;
  for (unsigned int i = 0; i < pData.size(); i++)
  {
    cout << pData[i] << endl;
  }

  cout << "Quality = " << getQuality() << endl;

  cout << "Start frequency = " << getStartFreq() << endl;
  cout << "End frequency = " << getEndFreq() << endl;
  cout << "Start time = " << getStartTime() << endl;
  cout << "End time = " << getEndTime() << endl;

}

string DH_Solution::createInsertStatement(TH_DB* th)
{
  ostringstream q;
  q.precision(PRECISION);
  if (itsWriteIndivParms) // Store parameters individually in a subtable
  {
    q << "INSERT INTO bbs3solutions (data, woid, iteration, fit, mu, stddev, chi, "
      << "startfreq, endfreq, starttime, endtime, hasconverged) VALUES ('";
    th->addDBBlob(this, q);
    q << "', "
      << getWorkOrderID() << ", "
      << getIteration() << ", "
      << getQuality().itsFit << ", "
      << getQuality().itsMu << ", "
      << getQuality().itsStddev << ", "
      << getQuality().itsChi << ", "
      << getStartFreq() << ", "
      << getEndFreq() << ", "
      << getStartTime() << ", "
      << getEndTime() << ", "
      << hasConverged() << ");";
    // store all parameters
    vector<ParmData> pSols;
    getSolution(pSols);
    for (uint i=0; i<pSols.size(); i++)
    {
      int nrCoeffs = pSols[i].getValues().nelements();
      q << "INSERT INTO "
	<< itsParmTableName 
	<< " (woid, iteration, parmname, nx, ny, coeff) VALUES ( "
	<< getWorkOrderID() << ", " 
	<< getIteration() << ", '"
	<< pSols[i].getName() << "', "
	<< pSols[i].nx() << ", "
	<< pSols[i].ny() << ", '{";
	for (int coeff=0; coeff<nrCoeffs; coeff++)
	{
	  q << pSols[i].getValue(coeff);
	  if (coeff < nrCoeffs-1)
	  {
	    q << ", ";
	  }
	}
	q << "}' ); ";
    }
  }
  else
  {
    q << "INSERT INTO bbs3solutions (data, woid, iteration, fit, mu, stddev, chi, "
      << "startfreq, endfreq, starttime, endtime, hasconverged) VALUES ('";
    th->addDBBlob(this, q);
    q << "', "
      << getWorkOrderID() << ", "
      << getIteration() << ", "
      << getQuality().itsFit << ", "
      << getQuality().itsMu << ", "
      << getQuality().itsStddev << ", "
      << getQuality().itsChi << ", "
      << getStartFreq() << ", "
      << getEndFreq() << ", "
      << getStartTime() << ", "
      << getEndTime() << ", "
      << hasConverged() <<");";
  }
  return q.str();
}

} // namespace LOFAR
