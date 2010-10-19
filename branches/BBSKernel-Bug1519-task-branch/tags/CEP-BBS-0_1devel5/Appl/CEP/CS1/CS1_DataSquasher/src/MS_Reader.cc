/***************************************************************************
 *   Copyright (C) 2007 by Adriaan Renting, ASTRON                         *
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
#include <casa/BasicMath/Math.h>
#include <casa/Arrays.h>
//#include <casa/Quanta/MVTime.h>

#include <iostream>

#include "MS_Reader.h"

using namespace casa;

//===============>>>  MS_Reader::MS_Reader  <<<===============

MS_Reader::MS_Reader(const std::string& msname)
{
  MSName = msname;
  MS     = new MeasurementSet(MSName, Table::Update);
  init();
}

//===============>>>  MS_Reader::~MS_Reader  <<<===============

MS_Reader::~MS_Reader()
{
  delete MS;
}

//===============>>> MS_Reader::init  <<<===============

void MS_Reader::init()
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

//===============>>> MS_Reader::BaselineIterator  <<<===============

MSAntenna MS_Reader::antenna()
{
  return (*MS).antenna();
}

//===============>>> MS_Reader::BaselineIterator  <<<===============

TableIterator MS_Reader::TimeslotIterator()
{
  Block<String> ms_iteration_variables(1);
  ms_iteration_variables[0] = "TIME_CENTROID";

  return TableIterator((*MS), ms_iteration_variables);
}

//===============>>> MS_Reader::BaselineIterator  <<<===============

TableIterator MS_Reader::TimeAntennaIterator()
{
  Block<String> ms_iteration_variables(4);
  ms_iteration_variables[0] = "TIME_CENTROID";
  ms_iteration_variables[1] = "DATA_DESC_ID";
  ms_iteration_variables[2] = "ANTENNA1";
  ms_iteration_variables[3] = "ANTENNA2";

  return TableIterator((*MS), ms_iteration_variables);
}

//===============>>> MS_Reader::WriteDataPointFlags  <<<===============

void   MS_Reader::WriteDataPointFlags(TableIterator* flag_iter,
                                    Matrix<Bool>* Flags,
                                    Bool FlagCompleteRow,
                                    bool ExistingFlags)
{
  Table         FlagTable = (*flag_iter).table();
//  ROTableVector<Int>      antenna1(FlagTable, "ANTENNA1");
//  ROTableVector<Int>      antenna2(FlagTable, "ANTENNA2");
//  ROTableVector<Int>      bandnr  (FlagTable, "DATA_DESC_ID");
//  ROTableVector<Double>   time    (FlagTable, "TIME_CENTROID");
  ArrayColumn  <Bool>     flags   (FlagTable, "FLAG");
  ScalarColumn <Bool>     flag_row(FlagTable, "FLAG_ROW");
//  ArrayColumn<String> flag_desc((*TimeslotTable), "FLAG_CATEGORY");
  if (ExistingFlags)
  {
    Matrix<Bool> OldFlags;
    Bool         OldFlagCompleteRow;
    flags.get(0, OldFlags);
    flag_row.get(0, OldFlagCompleteRow);
    flags.put(0, OldFlags || (*Flags));
  }
  else
  {
    flags.put(0, (*Flags));
  }
  flag_row.put(0, FlagCompleteRow);
}
//===============>>> MS_Reader  <<<===============

