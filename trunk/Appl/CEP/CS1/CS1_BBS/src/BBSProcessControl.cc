/***************************************************************************
 *   Copyright (C) 2006 by ASTRON, Adriaan Renting                         *
 *   renting@astron.nl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <lofar_config.h>

#include <CS1_BBS/BBSProcessControl.h>

#include <BBS/Solver.h>
#include <ParmDB/ParmDB.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>

#include <casa/Exceptions/Error.h>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace LOFAR::ParmDB;

namespace LOFAR 
{
  namespace CS1
  {
   //===============>>> BBSProcessControl::predict  <<<==============================
    void BBSProcessControl::predict (Prediffer& prediffer, const MSDesc& msd,
                                     const StepProp& stepProp,
                                     double timeStep, int startChan, int endChan)
    {
      double time = msd.startTime;
      double endTime = msd.endTime;
      while (time < endTime) {
        prediffer.setWorkDomain (startChan, endChan, time, time+timeStep);
        prediffer.setStepProp (stepProp);
        prediffer.writePredictedData();
        time += timeStep;
      }
    }           
    
    //===============>>> BBSProcessControl::substract  <<<==============================
    void BBSProcessControl::subtract (Prediffer& prediffer, const MSDesc& msd,
                                      const StepProp& stepProp,
                                      double timeStep, int startChan, int endChan)
    {
      double time = msd.startTime;
      double endTime = msd.endTime;
      while (time < endTime) {
        prediffer.setWorkDomain (startChan, endChan, time, time+timeStep);
        prediffer.setStepProp (stepProp);
        prediffer.subtractData();
        time += timeStep;
      }
    }           
    
    //===============>>> BBSProcessControl::predict  <<<==============================
    void BBSProcessControl::solve (Prediffer& prediffer, const MSDesc& msd,
                                  const StepProp& stepProp,
                                  double timestep, int startchan, int endchan,
                                  const SolveProp& solveProp,
                                  const vector<int32>& nrinterval,
                                  bool savesolution)
    {
      double time = msd.startTime;
      double endTime = msd.endTime;
      SolveProp solProp(solveProp);
      while (time < endTime) 
      {
        // Use given channels and time steps.
        // NB. This version of setWorkDomain has the following arguments:
        //     startChan, endChan
        //     startTime, lengthTime (!)
        prediffer.setWorkDomain (startChan, endChan, time, timestep);
        prediffer.setStepProp (stepProp);
        // Form the solve domains.
        const MeqDomain& workDomain = prediffer.getWorkDomain();
        vector<MeqDomain> solveDomains;
        double freq = workDomain.startX();
        double stepf = (workDomain.endX() - workDomain.startX()) / nrinterval[0]; //freq
        double stept = (workDomain.endY() - workDomain.startY()) / nrinterval[1]; //time
        double sdTime = workDomain.startY();
        for (int i=0; i<nrinterval[1]; ++i) {
          double sdFreq = workDomain.startX();
          for (int j=0; j<nrinterval[0]; ++j) {
            solveDomains.push_back (MeqDomain(sdFreq, sdFreq+stepf,
                                              sdTime, sdTime+stept));
            sdFreq += stepf;
          }
          sdTime += stept;
        }
        solProp.setDomains (solveDomains);
        prediffer.setSolveProp (solProp);
 
        Solver solver;
        solver.initSolvableParmData (1, solveDomains, prediffer.getWorkDomain());
        solver.setSolvableParmData (prediffer.getSolvableParmData(), 0);
        prediffer.showSettings();
        cout << "Before: " << setprecision(10)
             << solver.getSolvableValues(0) << endl;
    
        for(int i=0; i<solProp.getMaxIter(); ++i) {
          // Get the fitter data from the prediffer and give it to the solver.
          vector<casa::LSQFit> fitters;
          prediffer.fillFitters (fitters);
          solver.mergeFitters (fitters, 0);
    
          // Do the solve.
          solver.solve(false);
          cout << "iteration " << i << ":  " << setprecision(10)
               << solver.getSolvableValues(0) << endl;
          cout << solver.getQuality(0) << endl;
    
          prediffer.updateSolvableParms (solver.getSolvableParmData());
        }
        if (saveSolution) {
          cout << "Writing solutions into ParmDB ..." << endl;
          prediffer.writeParms();
        }
        time += timestep;
      }
    }


    //===============>>> BBSProcessControl::BBSProcessControl  <<<===============
    BBSProcessControl::BBSProcessControl()
    : ProcessControl()
    {
    }

    //===============>>> BBSProcessControl::~BBSProcessControl  <<<==============
    BBSProcessControl::~BBSProcessControl()
    {
    }

    //===============>>> BBSProcessControl::define  <<<==============================
    tribool BBSProcessControl::define()
    { 
      LOFAR::ACC::APS::ParameterSet* parameters = LOFAR::ACC::APS::globalParameterSet();
      // Read & parse parameters
      try
      {
        user            = parameters->getString ("user");
        instrumentPDB   = parameters->getString ("instrumentPDB");
        skyPDB          = parameters->getString ("skyPDB");
        measurementSet  = parameters->getString ("measurementSet");
        instrumentModel = parameters->getString ("instrumentModel");
        calcUVW         = parameters->getBool ("calcUVW");
        operation       = parameters->getString ("operation");
        columnNameIn    = parameters->getString ("columnNameIn");
        columnNameOut   = parameters->getString ("columnNameOut");
        timeDomainSize  = parameters->getDouble ("timeDomainSize");
        startChan       = parameters->getInt32 ("startChan");
        endChan         = parameters->getInt32 ("endChan");
        solvParms       = parameters->getStringVector ("solvParms");
        exclParms       = parameters->getStringVector ("exclParms");
        antennas        = parameters->getInt32Vector ("antennas");
        corrs           = parameters->getBoolVector ("corrs");
        nrSolveInterval = parameters->getInt32Vector ("nrSolveInterval");
        nriter          = parameters->getInt32 ("nriter");
        saveSolution    = parameters->getBool ("saveSolution");
      }
      catch (exception& _ex)
      {
        cout << "Parameter read or parse error: " << _ex.what() << endl;
        return false;
      }
      
      cout << "user                   : " << user << endl;
      cout << "instrument ParmDB      : " << instrumentPDB << endl;
      cout << "sky ParmDB             : " << skyPDB << endl;
      cout << "measurement set        : " << measurementSet << endl;
      cout << "instrument model       : " << instrumentModel << endl;
      cout << "calculate UVW          : " << calcUVW << endl;
      cout << "start channel          : " << startChan << endl;
      cout << "end channel            : " << endChan << endl;
      cout << "operation              : " << operation << endl;
      cout << "time domain size       : " << timeDomainSize << endl;
      
      return true;
    }

    //===============>>> BBSProcessControl::run  <<<=================================
    tribool BBSProcessControl::run()
    { 
      std::cout << "Runnning BBS please wait..." << std::endl;
      // Get meta data from description file.
      string name(measurementSet+"/vis.des");
      std::ifstream istr(name.c_str());
      ASSERTSTR (istr, "File " << measurementSet
                << "/vis.des could not be opened");
      BlobIBufStream bbs(istr);
      BlobIStream bis(bbs);
      MSDesc msd;
      bis >> msd;
      try {
        // Construct prediffer.
        Prediffer prediffer(measurementSet, 
                            ParmDBMeta("aips", instrumentPDB),
                            ParmDBMeta("aips", skyPDB),
                            0,
                            calcUVW);
        // Set strategy.
        StrategyProp stratProp;
        stratProp.setAntennas (antennas);
        stratProp.setCorr (corrs);
        stratProp.setInColumn (columnNameIn);
        ASSERT (prediffer.setStrategyProp (stratProp));
        // Fill step properties.
        StepProp stepProp;
        stepProp.setModel (StringUtil::split(instrumentModel,'.'));
        stepProp.setOutColumn (columnNameOut);
        
        if (operation == "solve") {
          ASSERT (nrSolveInterval.size()==2);
          ASSERT (nrSolveInterval[0] > 0  &&  nrSolveInterval[1] > 0);
          cout << "input column name      : " << columnNameIn << endl;
          cout << "solvable parms         : " << solvParms << endl;
          cout << "solvable parms excluded: " << exclParms << endl;
          cout << "solve nrintervals      : " << nrSolveInterval << endl;
          cout << "solve nriter           : " << nriter << endl;
          SolveProp solveProp;
          solveProp.setParmPatterns (solvParms);
          solveProp.setExclPatterns (exclParms);
          solveProp.setMaxIter (nriter);
          solve (prediffer, msd, stepProp,
                 timeDomainSize, startChan, endChan,
                 solveProp,
                 nrSolveInterval, saveSolution);
        } else if (operation == "predict") {
          cout << "output column name     : " << columnNameOut << endl;
          predict (prediffer, msd, stepProp,
                   timeDomainSize, startChan, endChan);
        } else if (operation == "subtract") {
          cout << "input column name      : " << columnNameIn << endl;
          cout << "output column name     : " << columnNameOut << endl;
          subtract (prediffer, msd, stepProp,
                    timeDomainSize, startChan, endChan);
        } else {
          cout << "Only operations solve, predict, and subtract are valid" << endl;
          return 1;
        }
      }
      catch (LOFAR::Exception& _ex)
      {
          cout << "LOFAR Exception caught: " << _ex.message() << endl;
          return false;
      }
      catch (exception& _ex)
      {
        cout << "General exception caught: " << _ex.what() << endl;
        return false;
      }
      catch (...)
      {
        cout << "Unknown error. " << endl;
        return false;
      }
      cout << "Program sucessfully ended" << endl;
      return true;
    }

    //===============>>> BBSProcessControl::init  <<<================================
    tribool BBSProcessControl::init()
    { return true;
    }

    //===============>>> BBSProcessControl::pause  <<<===============================
    tribool BBSProcessControl::pause(const std::string&)
    { return false;
    }

    //===============>>> BBSProcessControl::quit  <<<================================
    tribool BBSProcessControl::quit()
    { return true;
    }

    //===============>>> BBSProcessControl::recover  <<<=============================
    tribool BBSProcessControl::recover(const std::string&)
    { return false;
    }

    //===============>>> BBSProcessControl::reinit  <<<==============================
    tribool BBSProcessControl::reinit(const  std::string&)
    { return false;
    }

    //===============>>> BBSProcessControl::askInfo  <<<=============================
    std::string BBSProcessControl::askInfo(const std::string&)
    { return std::string("");
    }

    //===============>>> BBSProcessControl::snapshot  <<<============================
    tribool BBSProcessControl::snapshot(const std::string&)
    { return false;
    }
  } //namespace CS1
}; //namespace LOFAR
