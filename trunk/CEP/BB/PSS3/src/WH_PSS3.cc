//  WH_PSS3.cc:
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

#include <stdio.h>             // for sprintf

#include <PSS3/WH_PSS3.h>
#include <CEPFrame/Step.h>
#include <Common/Debug.h>
#include <PSS3/DH_WorkOrder.h>
#include <PSS3/DH_Solution.h>
#include <PSS3/CalibratorOld.h>
#include <PSS3/Strategy.h>

namespace LOFAR
{

const int DefaultAntennaCount = 21;

const int MaxNumberOfParms = 30;

WH_PSS3::WH_PSS3 (const string& name, const string & msName, 
		  const string& meqModel, const string& skyModel,
		  const string& dbType, const string& dbName,
		  const string& dbHost,
		  const string& dbPwd, unsigned int ddid, 
		  const string& modelType, bool calcUVW, 
		  const string& dataColName, 
		  const string& residualColName, bool outputAllIter, 
		  int number)
  : WorkHolder        (2, 2, name, "WH_PSS3"),
    itsCal            (0),
    itsMSName         (msName),
    itsMeqModel       (meqModel),
    itsSkyModel       (skyModel),
    itsDbType         (dbType),
    itsDbName         (dbName),
    itsDbHost         (dbHost),
    itsDbPwd          (dbPwd),
    itsDDID           (ddid),
    itsModelType      (modelType),
    itsCalcUVW        (calcUVW),
    itsDataColName    (dataColName),
    itsResidualColName(residualColName),
    itsOutputAllIter  (outputAllIter),
    itsNumber         (number)

{
  TRACER4("WH_PSS3 construction");
  getDataManager().addInDataHolder(0, new DH_WorkOrder(name+"_in"));
  getDataManager().addInDataHolder(1, new DH_Solution("in_1"));
  getDataManager().addOutDataHolder(0, new DH_WorkOrder(name+"_out"));
  getDataManager().addOutDataHolder(1, new DH_Solution("out_1"));
  // switch input and output channel trigger off
  getDataManager().setAutoTriggerIn(0, false);
  getDataManager().setAutoTriggerIn(1, false);
  getDataManager().setAutoTriggerOut(0, false);
  getDataManager().setAutoTriggerOut(1, false);

  for (int i = 0; i < DefaultAntennaCount; i ++)
  {
    itsAnt1.push_back (4 * i);
    itsAnt2.push_back (4 * i);
  }
}

WH_PSS3::~WH_PSS3()
{
  TRACER4("WH_PSS3 destructor");
  if (itsCal != 0)    
  {                  
    delete itsCal;   
  }                  
}


WH_PSS3* WH_PSS3::make (const string& name)
{
  return new WH_PSS3 (name, itsMSName, itsMeqModel, itsSkyModel, itsDbType,
		      itsDbName, itsDbHost, itsDbPwd,  itsDDID, itsModelType, 
		      itsCalcUVW, itsDataColName, itsResidualColName, 
		      itsOutputAllIter, itsNumber);
}

void WH_PSS3::preprocess()
{
  TRACER4("WH_PSS3 preprocess()");
}

void WH_PSS3::process()
{
  // Create a Calibrator object
  TRACER4("WH_PSS3 process()");
  itsCal = new CalibratorOld(itsMSName, itsMeqModel, itsSkyModel, itsDbType, 
			     itsDbName, itsDbHost, itsDbPwd);

  // Query the database for a work order
  DH_WorkOrder* wo =  dynamic_cast<DH_WorkOrder*>(getDataManager().getInHolder(0));
  AssertStr(wo !=  0, "Dataholder is not a work order");
  DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
  AssertStr(woPtr != 0, "Input work order cannot be cast to a DH_PL");
 
  // Wait for workorder
  bool firstTime = true;
  while ((woPtr->queryDB("status=0 and (kstype='KS' or kstype='" + getName()
			 + "') order by kstype desc, woid asc")) <= 0)
  {
    if (firstTime)
    {
      cout << "No workorder found by " << getName() << ". Waiting for work order..." << endl;
      firstTime = false;
    }
  }

  // Update workorder status
  wo->setStatus(DH_WorkOrder::Assigned);
  woPtr->updateDB();

  wo->dump();
  
  int argsSize = wo->getArgsSize();
  char data[argsSize];
  vector<string> pNames;    // Parameter names
  vector<int> solNumbers;   // Solution IDs
  wo->getVarData(data, pNames, solNumbers);
  
  // Create a strategy object
  TRACER1("Strategy number: " << wo->getStrategyNo());
  Strategy strat(wo->getStrategyNo(), itsCal, argsSize, data);
  
  // Get solution dataholder DH_Solution* sol;
  DH_Solution* sol = dynamic_cast<DH_Solution*>(getDataManager().getInHolder(1));
  AssertStr(sol != 0, "Dataholder can not be cast to a DH_Solution");
  sol->clearData();
  DH_PL* solPtr = dynamic_cast<DH_PL*>(sol);
  AssertStr(solPtr != 0, "Input solution cannot be cast to a DH_PL");      
  
  int noStartSols = wo->getNoStartSolutions();
  if (noStartSols > 0)
  { 
    vector<string> allNames(0);      // Contains all start parameters
    vector<double> allValues(0);     // Contains all start values

    for (int i=0; i<noStartSols; i++)
    {
      // Query the database for the start solution
      cout << "Use start solution number " << solNumbers[i] << endl;
      std::ostringstream q;
      q << "bbid=" << solNumbers[i];
      cout << q.str() << endl;
      bool firstGo = true;
      // Wait for solution
      while (solPtr->queryDB(q.str()) <= 0)
      {
	if (firstGo)
	{
	  firstGo = false;
	  cout << "No start solution " << solNumbers[i] << " found by "
	       << getName() << ". Waiting..." << endl;
	}
      }  
      AssertStr(solPtr->queryDB(q.str())==1,
		"Multiple solutions with ID " << solNumbers[i] << " found in database.");
      vector<string> startNames;
      vector<double> startValues; 
      sol->getSolution(startNames, startValues);
      addToStartVectors(startNames, startValues, allNames, allValues); 
    }
    vector<int> startSources;
    getSourceNumbersFromNames(allNames, startSources);

    // Use start solution in the strategy
    strat.useParms(allNames, allValues, startSources);
  }

  vector<string> resPNames;
  vector<double> resPValues;
  int iterNo = -1;
  int count = 0;
  Quality resQuality;
  resQuality.init();
  // Show parameter names
  for (unsigned int i = 0; i < pNames.size(); i++)
  {  
    TRACER1("Parameter name " << i << " :" << pNames[i]);
  }
  
  // Execute the strategy until finished with all iterations, intervals 
  // and sources.
  while (strat.execute(pNames, resPNames, resPValues, resQuality, iterNo))
  {
    TRACER1("Executed strategy");
    if (itsOutputAllIter)   // Write every found solution in DH_Solution
    {    
      sol->setSolution(resPNames, resPValues);
      sol->setIterationNo(iterNo);
      sol->setQuality(resQuality);
      sol->setID(itsNumber++);
      sol->setWorkOrderID(wo->getWorkOrderID());
      wo->setStatus(DH_WorkOrder::Executed);
      // Add solution to database and update work order
      solPtr->insertDB();
      woPtr->updateDB();
      // Dump to screen
      sol->dump();
    }
    count++;
    
  }

  if (!itsOutputAllIter) // Write only the resulting output
  {  
    sol->setSolution(resPNames, resPValues);
    sol->setIterationNo(iterNo);
    sol->setQuality(resQuality);
    sol->setID(itsNumber++);
    sol->setWorkOrderID(wo->getWorkOrderID());
    wo->setStatus(DH_WorkOrder::Executed);
    // Add solution to database and update work order
    solPtr->insertDB();
    woPtr->updateDB();
  }

  delete itsCal;
}

void WH_PSS3::dump()
{
}

void WH_PSS3::addToStartVectors(const vector<string>& names,
				const vector<double>& values,
				vector<string>& allNames,
				vector<double>& allValues) const
{
  Assert(names.size() == values.size());
  unsigned int noParms = allNames.size();
  for (unsigned int i=0; i<names.size(); i++)
  {
    bool nameFound = false;
    for (unsigned int j=0; j<noParms; j++)
    {
      if (names[i] == allNames[j])
      {
	allValues[j] = values[i]; // Replace value
	nameFound = true;
	break;
      }
    }
    if (!nameFound)
    {
      allNames.push_back(names[i]);  // Add name and value
      allValues.push_back(values[i]);
    }
  }
}

void WH_PSS3::getSourceNumbersFromNames(vector<string>& names, vector<int>& srcNumbers) const
{
  int sourceNo;
  bool srcFound;
  int numberOfNames = names.size();
  for (int index = 0; index < numberOfNames; index++)
  {
    sourceNo = -1;
    string::size_type dotPos;
    dotPos = names[index].find_last_of(".");  // Find last "."
    if (dotPos != string::npos)
    {
      string::size_type pos = names[index].find("CP", dotPos+1); // Find "CP"
      if (pos != string::npos)
      {
	string sourceStr = names[index].substr(pos+2, 10); // Get the source number
	sourceNo = atoi(sourceStr.c_str()); 

	srcFound = false;
	// Check if source number has already been added to the vector
	for (unsigned int src = 0; src < srcNumbers.size(); src++)
	{
	  if (srcNumbers[src] == sourceNo)
	  { 
	    srcFound = true;
	    break;
	  } 
	}	
	if (srcFound == false)
	{
	  srcNumbers.push_back(sourceNo);
	}
      }
    }
  }
   
}



} // namespace LOFAR
