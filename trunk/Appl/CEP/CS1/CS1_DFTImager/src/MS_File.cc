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
#include <casa/BasicMath/Math.h>
#include <casa/Arrays.h>
//#include <casa/Quanta/MVTime.h>

#include <iostream>

#include "MS_File.h"


using namespace casa;

namespace LOFAR 
{
  namespace CS1
  {
    
    //===============>>>  MS_File::MS_File  <<<===============
    
    MS_File::MS_File(const std::string& msname)
    {
      MSName = msname;
      MS     = new MeasurementSet(MSName, Table::Update);
      init();
    }
    
    //===============>>>  MS_File::~MS_File  <<<===============
    
    MS_File::~MS_File()
    {
      delete MS;
    }
    
    //===============>>> MS_File::init  <<<===============
    
    void MS_File::init()
    {
      //Number of samples
      itsNumSamples                    = (*MS).nrow();    
      std::cout << "NumSamples" << itsNumSamples << std::endl;
      //Number of Fields
      MSField fields                   = (*MS).field();
      itsNumFields                     = fields.nrow();
      std::cout << "NumFields" << itsNumFields << std::endl;
      
      //Number of Antennae    
      MSAntenna antennae               = (*MS).antenna();
      itsNumAntennae                   = antennae.nrow();
      std::cout << "NumAntennae" << itsNumAntennae << std::endl;
      
      //Antenna Names
      ROScalarColumn<String>           ANT_NAME_col(antennae, "NAME");
      Vector<String>         ant_names = ANT_NAME_col.getColumn();
      ant_names.tovector(itsAntennaNames);
          
      //Number of channels in the Band
      MSSpectralWindow spectral_window = (*MS).spectralWindow();
      ROScalarColumn<Int>              NUM_CHAN_col(spectral_window, "NUM_CHAN");
      itsNumChannels                   = NUM_CHAN_col(0);
      std::cout << "NumChannels" << itsNumChannels << std::endl;
      
      //Number of polarizations
      MSPolarization      polarization = (*MS).polarization();
      ROScalarColumn<Int>              NUM_CORR_col(polarization, "NUM_CORR");
      itsNumPolarizations              = NUM_CORR_col(0);
      ROArrayColumn<Int>               CORR_TYPE_col(polarization, "CORR_TYPE");
      itsPolarizations.resize(itsNumPolarizations);
      CORR_TYPE_col.get(0, itsPolarizations);
      std::cout << "NumPolarizations" << itsNumPolarizations << std::endl;
      
      //calculate theoretical noise level
      ROScalarColumn<Double>           EXPOSURE_col((*MS), "EXPOSURE");
      Double exposure                  = EXPOSURE_col(0);
      
      ROScalarColumn<Double>           TOTAL_BANDWIDTH_col(spectral_window, "TOTAL_BANDWIDTH");
      Double bandwidth                 = TOTAL_BANDWIDTH_col(0) / itsNumChannels;
      
      itsNoiseLevel                    = 1.0 / sqrt(bandwidth * exposure);
      std::cout << "Noiselevel" << itsNoiseLevel << std::endl;
          
      //calculate number of timeslots
      ROScalarColumn<Double>           INTERVAL_col((*MS), "INTERVAL");
      Double interval                  = INTERVAL_col(0);
      
      //Number of timeslots
      ROScalarColumn<Double>            TIME_CENTROID_col((*MS), "TIME_CENTROID");
      Double firstdate                 = TIME_CENTROID_col(0);
      Double lastdate                  = TIME_CENTROID_col(itsNumSamples-1);
      std::cout << "interval" << interval << std::endl;
      
      itsNumTimeslots                  = (int)((lastdate-firstdate)/interval) + 1;
      std::cout << "Numtimeslots" << itsNumTimeslots << std::endl;
      
      //calculate number of baselines.
      itsNumPairs = (itsNumAntennae) * (itsNumAntennae + 1) / 2; //Triangular numbers formula
      std::cout << "NumPairs" << itsNumPairs << std::endl;
      
      //calculate number of Bands
      itsNumBands                      = itsNumSamples / (itsNumPairs * itsNumTimeslots);
      std::cout << "NumBands" << itsNumBands << std::endl;
      
    }
    
    //===============>>> MS_File::antennas  <<<===============
    
    MSAntenna MS_File::antenna()
    {  
      return (*MS).antennas();
    }
    
    //===============>>> MS_File::BaselineIterator  <<<===============
    
    TableIterator MS_File::TimeslotIterator()
    {  
      Block<String> ms_iteration_variables(1);
      ms_iteration_variables[0] = "TIME_CENTROID";
      
      return TableIterator((*MS), ms_iteration_variables);
    }  
    
    //===============>>> MS_File::BaselineIterator  <<<===============
    
    TableIterator MS_File::TimeAntennaIterator()
    {  
      Block<String> ms_iteration_variables(4);
      ms_iteration_variables[0] = "TIME_CENTROID";
      ms_iteration_variables[1] = "DATA_DESC_ID";
      ms_iteration_variables[2] = "ANTENNA1";
      ms_iteration_variables[3] = "ANTENNA2";
      
      return TableIterator((*MS), ms_iteration_variables);
    }  
    //===============>>> MS_File  <<<===============
  }
}