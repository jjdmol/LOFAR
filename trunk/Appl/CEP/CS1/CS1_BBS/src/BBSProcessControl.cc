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
#include <Common/VectorUtil.h>

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
                                    const string& columnName,
                                    double timeStep, int startChan, int endChan)
    {
      double time = msd.startTime;
      double endTime = msd.endTime;
      while (time < endTime) {
        prediffer.setWorkDomain (startChan, endChan, time, time+timeStep);
        prediffer.writePredictedData (columnName);
        time += timeStep;
      }
    }           
    
    //===============>>> BBSProcessControl::substract  <<<==============================
    void BBSProcessControl::subtract (Prediffer& prediffer, const MSDesc& msd,
                                      const string& columnNameIn, const string& columnNameOut,
                                      double timeStep, int startChan, int endChan)
    {
      double time = msd.startTime;
      double endTime = msd.endTime;
      while (time < endTime) {
        prediffer.setWorkDomain (startChan, endChan, time, time+timeStep);
        prediffer.subtractData (columnNameIn, columnNameOut, false);
        time += timeStep;
      }
    }           
    
    //===============>>> BBSProcessControl::predict  <<<==============================
    void BBSProcessControl::solve (Prediffer& prediffer, const MSDesc& msd,
                                  const string& columnname,
                                  double timestep, int startchan, int endchan,
                                  const vector<string>& solvparms, const vector<string>& exclparms,
                                  const vector<int32>& nrinterval,
                                  int maxiterations,
                                  bool savesolution)
    {
      prediffer.clearSolvableParms();
      prediffer.setSolvableParms (solvParms, exclParms);
      double time = msd.startTime;
      double endTime = msd.endTime;
      while (time < endTime) {
        // Use given channels and time steps.
        prediffer.setWorkDomain (startChan, endChan, time, time+timestep);
        // Form the solve domains.
        const MeqDomain& workDomain = prediffer.getWorkDomain();
        vector<MeqDomain> solveDomains;
        double freq = workDomain.startX();
        double stepf = (workDomain.endX() - freq) / nrinterval[0];
        double stept = (workDomain.endY() - time) / nrinterval[1];
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
        prediffer.initSolvableParms (solveDomains);
    
        Solver solver;
        solver.initSolvableParmData (1, solveDomains, prediffer.getWorkDomain());
        solver.setSolvableParmData (prediffer.getSolvableParmData(), 0);
        prediffer.showSettings();
        cout << "Before: " << setprecision(10)
            << solver.getSolvableValues(0) << endl;
        
        for(int i=0; i<maxiterations; ++i) {
          // Get the fitter data from the prediffer and give it to the solver.
          vector<casa::LSQFit> fitters;
          prediffer.fillFitters (fitters, columnname);
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
        user                = parameters->getString ("user");
        instrumentPDB       = parameters->getString ("instrument_parmdb");
        skyPDB              = parameters->getString ("sky_parmdb");
        measurementSet      = parameters->getString ("measurement_set");
        instrumentModelType = parameters->getString ("instrument_model");
        calcUVW             = parameters->getBool ("calculate_UVW");
        operation           = parameters->getString ("operation");
        columnNameIn        = parameters->getString ("data_column_in");
        columnNameOut       = parameters->getString ("data_column_out");
        timeDomainSize      = parameters->getDouble ("time_domain_size");
        startChan           = parameters->getInt32 ("start_channel");
        endChan             = parameters->getInt32 ("end_channel");
        solvParms           = parameters->getStringVector ("solvable_parms");
        exclParms           = parameters->getStringVector ("solvable_parms_excluded");
        nrSolveInterval     = parameters->getInt32Vector ("nr_solve_interval");
        nriter              = parameters->getInt32 ("nriter");
        saveSolution        = parameters->getBool ("save_solution");
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
      cout << "instrument model       : " << instrumentModelType << endl;
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
      vector<int> antennaSelector(msd.antNames.size());
      for (uint i=0; i<antennaSelector.size(); ++i) {
        antennaSelector[i] = i;
      }
      vector<vector<int> > sourceGroups;
      // Construct prediffer.
      Prediffer prediffer(measurementSet, 
                          ParmDBMeta("aips", instrumentPDB),
                          ParmDBMeta("aips", skyPDB),
                          antennaSelector, instrumentModelType, sourceGroups,
                          calcUVW);
      try {
        if (operation == "solve") {
          ASSERT (nrSolveInterval.size()==2);
          ASSERT (nrSolveInterval[0] > 0  &&  nrSolveInterval[1] > 0);
          cout << "input column name      : " << columnNameIn << endl;
          cout << "solvable parms         : " << solvParms << endl;
          cout << "solvable parms excluded: " << exclParms << endl;
          cout << "solve nrintervals      : " << nrSolveInterval << endl;
          cout << "solve nriter           : " << nriter << endl;
          solve (prediffer, msd, columnNameIn,
                timeDomainSize, startChan, endChan,
                solvParms, exclParms,
                nrSolveInterval, nriter, saveSolution);
        } else if (operation == "predict") {
          cout << "output column name     : " << columnNameOut << endl;
          predict (prediffer, msd, columnNameOut,
                  timeDomainSize, startChan, endChan);
        } else if (operation == "subtract") {
          cout << "input column name      : " << columnNameIn << endl;
          cout << "output column name     : " << columnNameOut << endl;
          subtract (prediffer, msd, columnNameIn, columnNameOut,
                    timeDomainSize, startChan, endChan);
        } else {
          cout << "Only operations solve, predict, and subtract are valid" << endl;
          return 1;
        }
      }
      catch (exception& _ex)
      {
        cout << "error: " << _ex.what() << endl;
        return false;
      }
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
