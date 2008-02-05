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
#include <tables/Tables.h>
#include <tables/Tables/TableIter.h>
#include "FrequencyFlagger.h"
#include <casa/Quanta/MVEpoch.h>

using namespace casa;

enum CorrelationTypes {None=0,I=1,Q=2,U=3,V=4,RR=5,RL=6,LR=7,LL=8,XX=9,XY=10,YX=11,YY=12}; //found somewhere in AIPS++, don't remember where

//===============>>>  itoa  <<<===============
//ANSI C++ doesn't seem to have a decent function for this, or I'm not aware of it. Need to rename it to IntToStr(), to avoid confusion
std::string itoa(int value, int base=10)
{ // found on http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
  //maybe copyright Robert Jan Schaper, Ray-Yuan Sheu, Rodrigo de Salvo Braz, Wes Garland and John Maloney
  enum { kMaxDigits = 35 };
  std::string buf;
  buf.reserve( kMaxDigits ); // Pre-allocate enough space.
        // check that the base if valid
  if (base < 2 || base > 16) return buf;
  int quotient = value;
        // Translating number to string with base:
  do {
    buf += "0123456789abcdef"[ std::abs( quotient % base ) ];
    quotient /= base;
  } while ( quotient );
        // Append the negative sign for base 10
  if ( value < 0 && base == 10) buf += '-';
  std::reverse( buf.begin(), buf.end() );
  return buf;
}
namespace LOFAR
{
  namespace CS1
  {
    //===============>>>  FrequencyFlagger::FrequencyFlagger  <<<===============
    /* initialize some meta data and get the datastorage the right size. */
    FrequencyFlagger::FrequencyFlagger(MS_File* InputMSfile,
                                       double InputThreshold)
    {
      MSfile           = InputMSfile;
      Threshold        = InputThreshold;
      NumAntennae      = (*MSfile).itsNumAntennae;
      NumPairs         = (*MSfile).itsNumPairs;
      NumBands         = (*MSfile).itsNumBands;
      NumChannels      = (*MSfile).itsNumChannels;
      NumPolarizations = (*MSfile).itsNumPolarizations;
      NoiseLevel       = (*MSfile).itsNoiseLevel;
      NumTimeslots     = (*MSfile).itsNumTimeslots;
      AntennaNames     = (*MSfile).itsAntennaNames;

      PairsIndex.resize(NumPairs);
      Statistics = Cube<int>(NumBands, NumAntennae, NumAntennae, 0);

      TimeslotData.resize(NumPolarizations, NumChannels, NumPairs*NumBands);
      Flags.resize(NumPolarizations, NumChannels, NumPairs*NumBands);

      int index = 0;
      for (int i = 0; i < NumAntennae; i++)
      { for(int j = i; j < NumAntennae; j++)
        { PairsIndex[index]           = pairii(i, j);
          BaselineIndex[pairii(i, j)] = index++;
        }
      }
      ComputeBaselineLengths();
    }

    //===============>>>  FrequencyFlagger::~FrequencyFlagger  <<<===============

    FrequencyFlagger::~FrequencyFlagger()
    {
    }

    //===============>>> FrequencyFlagger::ComputeBaselineLengths  <<<===============
    /* compute baseline lengths, and determine the longest one.*/
    void FrequencyFlagger::ComputeBaselineLengths()
    {
      MaxBaselineLength = 0.0;
      BaselineLengths.resize(NumPairs);
      //Antenna positions
      MSAntenna antenna     = (*MSfile).antenna();
      ROArrayColumn<Double>  position(antenna, "POSITION");
      for (int i = 0; i < NumAntennae; i++ )
      {
        for (int j = i; j < NumAntennae; j++)
        {
          Vector<Double> p(position(i) - position(j));
          double temp   = sqrt(p(0)*p(0) + p(1)*p(1) + p(2)*p(2));
          BaselineLengths[BaselineIndex[pairii(i, j)]] = temp;
          if (temp > MaxBaselineLength && temp < 3000000) //radius of the Earth in meters? WSRT sometimes has fake telescopes at 3854243
          { MaxBaselineLength = temp;                     // non-existent antenna's can have position (0,0,0)
          }
        }
      }
    }

    //===============>>> FrequencyFlagger::FlagDataOrBaselines  <<<===============
    /*This function outputs the gathered statistics.*/
    void FrequencyFlagger::ProcessStatistics()
    {
      vector<int>  bands(NumBands);
      vector<int>  antennae(NumAntennae);
      unsigned int namelength = 6;
      for(int i = 0; i < NumAntennae; i++)
      {
        if (namelength < AntennaNames[i].size())
        { namelength = AntennaNames[i].size();
        }
      }
      for (int i = 0; i < NumBands; i++)
      {
        cout << "Band: " << i+1 << endl;
        cout << string(namelength+1,' ');
        for(int j = 0; j < NumAntennae; j++)
        {
          string out = AntennaNames[j];
          out.resize(namelength+1,' ');
          cout << out;
        }
        cout << endl;
        for(int j = 0; j < NumAntennae; j++)
        {
          string out = AntennaNames[j];
          out.resize(namelength+1,' ');
          cout << out;
          for(int k = 0; k < NumAntennae; k++)
          {
            if (k < j) //We print a complete array, but we have inspected only those where k >= j
            {
              int val = 100 * Statistics(i,k,j) / (NumChannels * NumTimeslots * NumPolarizations);
              bands[i]    += val;
              antennae[j] += val;
              antennae[k] += val;
              string out = itoa(val) + "%";
              out.resize(namelength+1,' ');
              cout << out;
            }
            else
            {
              int val = 100 * Statistics(i,j,k) / (NumChannels * NumTimeslots * NumPolarizations);
              bands[i]    += val;
              antennae[j] += val;
              antennae[k] += val;
              string out = itoa(val) + "%";
              out.resize(namelength+1,' ');
              cout << out;
            }
              /*{ cout << rms << "Faulty baseline detected: Antenna " << j+1
                    << ":" << k+1 << " SpectralWindow: "<< i+1 << endl;
              }
            }*/
          }
          cout << endl;
        }
      }
      cout << "Bands (flagged %):    ";
      for (int i = 0; i < NumBands; i++)
      {
        string out = string("Band") + itoa(i);
        out.resize(namelength+1,' ');
        cout << out;
      }
      cout << endl << "                      ";
      for (int i = 0; i < NumBands; i++)
      {
        string out = itoa(bands[i] / (NumAntennae*NumAntennae)) + "%";
        out.resize(namelength+1,' ');
        cout << out;
      }
      cout << endl << "Antennae (flagged %): " ;
      for(int j = 0; j < NumAntennae; j++)
      {
        string out = AntennaNames[j];
        out.resize(namelength+1,' ');
        cout << out;
      }
      cout << endl << "                       ";
      for (int i = 0; i < NumAntennae; i++)
      {
        string out = itoa(antennae[i] / (NumBands*NumAntennae*2)) + "%";
        out.resize(namelength+1,' ');
        cout << out;
      }
      cout << endl;
    }
    //===============>>> FrequencyFlagger::FlagTimeslot  <<<===============
    /* This function inspects each visibility in a cetain baseline-band
    and flags on complexe distance, then determines to flag the entire baseline-band
    based on the RMS of the points it didn't flag.*/
    bool FrequencyFlagger::FlagBaselineBand(Matrix<Bool> Flags,
                                            Matrix<Complex> Timeslots,
                                            int* flagCounter,
                                            double FlagThreshold)
    {
      vector<double> MS(NumPolarizations, 0.0);
      vector<double> RMS(NumPolarizations);
      vector<int>    RMSCounter(NumPolarizations, 0);
      bool           FlagCompleteRow = true;
      int            flagcount       = 0;
      for (int j = NumPolarizations-1; j >= 0; j--)
      {
        for (int i = NumChannels-1; i >= 0; i--) //calculata RMS of unflagged datapoints
        {
          if (!ExistingFlags || !Flags(j, i))
          { double temp = pow(abs(Timeslots(j, i)), 2);
            if (!isNaN(temp))
            {
              MS[j] += temp;
              RMSCounter[j] += 1;
            }
          }
        }
        if (RMSCounter[j])
        { RMS[j] = sqrt(MS[j] /RMSCounter[j]);
          for (int i = NumChannels-1; i >= 0; i--)
          {
            if (!ExistingFlags || !Flags(j, i))
            { double temp = abs(Timeslots(j, i));
              bool flag   = isNaN(temp) || RMS[j] * FlagThreshold < temp;
              //cout << RMS[j] << " " << (RMS[j] * FlagThreshold) << " " << temp << " " << (flag) << endl;
              if (flag)
              { flagcount++;
              }
              else
              { FlagCompleteRow = false;
              }
              Flags(j, i) = flag || (ExistingFlags && Flags(j, i));
            }
          }
        }
        else
        {
          flagcount += NumChannels;
          Flags.row(j) = true;
        }
      }
      //these need to be separated out into a different function for clarity
      if (flagcount > 0.9 * NumChannels * NumPolarizations) //more as 90% bad
      {
        FlagCompleteRow = true;
        Flags           = true;
        flagcount       = NumChannels * NumPolarizations;
      }
      (*flagCounter) += flagcount;
      return FlagCompleteRow;
    }
    //===============>>> FrequencyFlagger::FlagBaseline  <<<===============
    /* This function iterates over baseline and band and uses FlagBaselineBand() to determine
      for each one if it needs to be flagged. It treats the autocorrelations separately,
      to detect entire malfunctioning telescopes. Finally it writes the flags.
    */
    void FrequencyFlagger::FlagTimeslot(TableIterator* flag_iter,
                                            bool ExistingFlags)
    {
      vector<bool>   FlagCompleteRow(NumPairs*NumBands);
      vector<int>    stats(NumPairs*NumBands);
      for (int i = 0; i < NumBands; i++)
      {
        for(int j = 0; j < NumAntennae; j++)
        {
          for(int k = j; k < NumAntennae; k++)
          {
            int index = i*NumPairs + BaselineIndex[pairii(j, k)];
            if ((BaselineLengths[BaselineIndex[pairii(j, k)]] < 3000000))//radius of the Earth in meters? WSRT sometimes has fake telescopes at 3854243 m
            { //we skip the non-existent telescopes
              FlagCompleteRow[index] = FlagBaselineBand(Flags.xyPlane(index),
                                                        TimeslotData.xyPlane(index),
                                                        &(stats[index]),
                                                        Threshold);
            }
          }
        }
        //this code could be separated in to different functions
        for(int j = 0; j < NumAntennae; j++) //write the data
        {
          for(int k = j; k < NumAntennae; k++)
          {
            int index = i*NumPairs + BaselineIndex[pairii(j, k)];
            Matrix<Bool> flags = Flags.xyPlane(index);
            if (FlagCompleteRow[index])
            {
              Matrix<Bool> flags     = Flags.xyPlane(index);
              flags                  = true;
              Statistics(i,j,k)     += NumChannels * NumPolarizations;
            }
            else
            { Statistics(i,j,k)     += stats[index];
            }
            (*MSfile).WriteDataPointFlags(flag_iter, &flags, FlagCompleteRow[index], ExistingFlags);
            (*flag_iter)++;
          }
        }
      }
    }

    //===============>>> FrequencyFlagger::UpdateTimeslotData  <<<===============
    /* This function reads the visibility data for one timeslot and checks if
      a mosaicing mode is active*/
    /* The datastructure Timeslotdata is rather complex because it flattens
      half-filled antenna x antenna x bands matrix into a vector of NumPairs X bands length. */
    bool FrequencyFlagger::UpdateTimeslotData(vector<int>* OldFields,
                                                  vector<int>* OldBands,
                                                  int* TimeCounter,
                                                  Table* TimeslotTable,
                                                  double* Time)
    {
      int           rowcount = (*TimeslotTable).nrow();
      ROTableVector<Int>     antenna1((*TimeslotTable), "ANTENNA1");
      ROTableVector<Int>     antenna2((*TimeslotTable), "ANTENNA2");
      ROTableVector<Int>     bandnr  ((*TimeslotTable), "DATA_DESC_ID");
      ROTableVector<Int>     fieldid ((*TimeslotTable), "FIELD_ID");
      ROTableVector<Double>  time    ((*TimeslotTable), "TIME_CENTROID");//for testing purposes
      ROArrayColumn<Complex> data    ((*TimeslotTable), "DATA");
      ROArrayColumn<Bool>    flags    ((*TimeslotTable), "FLAG");
      Cube<Complex>          Data(NumPolarizations, NumChannels, rowcount);
      Cube<Bool>             Flags(NumPolarizations, NumChannels, rowcount);

      data.getColumn(Data); //We're not checking Data.nrow() Data.ncolumn(), assuming all data is the same size.
      flags.getColumn(Flags); //We're not checking Data.nrow() Data.ncolumn(), assuming all data is the same size.
      (*Time) = time(0);//for testing purposes, might be useful in the future

      bool NewField_or_Frequency = false;

      for (int i = 0; i < rowcount; i++)
      {
        int bi    = BaselineIndex[pairii(antenna1(i), antenna2(i))];
        int field = fieldid(i);
        int band  = bandnr(i);
        int index = (band % NumBands) * NumPairs + bi;
        if ((*TimeCounter))
        {
          if (field != (*OldFields)[bi]) //pointing mosaicing
          { NewField_or_Frequency = true;
          }
          if (band  != (*OldBands)[index]) //frequency mosaicing
          { NewField_or_Frequency = true;
          }
        }
        (*OldFields)[bi]   = field;
        (*OldBands)[index] = band;

        TimeslotData.xyPlane(index) = Data.xyPlane(i);
        Flags.xyPlane(index)    = Flags.xyPlane(i); //TimeslotData is only WindowSize size!
      }
      return NewField_or_Frequency; //assuming everybody is changing field or frequency at the same time!
    }

    //===============>>> FrequencyFlagger::FlagDataOrBaselines  <<<===============
    /* This function iterates over the data per timeslot and uses Flagtimeslot()
      to actually flag datapoints (if flagDatapoints), and entire baselines (if flagRMS)*/
    void FrequencyFlagger::FlagDataOrBaselines(bool existing)
    {
      TableIterator timeslot_iter = (*MSfile).TimeslotIterator();
      TableIterator flag_iter     = (*MSfile).TimeAntennaIterator();
      double        Time          = 0.0;//for testing purposes
      vector<int>   OldFields(NumPairs);         //to check on multipointing and mosaicing
      vector<int>   OldBands(NumPairs*NumBands); //to check on multifrequency and freq mosaicing
      ExistingFlags = existing;
      int           step          = NumTimeslots / 10 + 1; //not exact but it'll do
      int           row           = 0;

      while (!timeslot_iter.pastEnd())
      {
        Table TimeslotTable  = timeslot_iter.table();
        bool  NewFieldorFreq = UpdateTimeslotData(&OldFields,
                                                  &OldBands,
                                                  &row,
                                                  &TimeslotTable,
                                                  &Time);
        //cout << "Processing: " << MVTime(Time/(24*3600)).string(MVTime::YMD) << endl; //for testing purposes

        FlagTimeslot(&flag_iter, ExistingFlags);
        timeslot_iter++;
        if (row++ % step == 0) // to tell the user how much % we have processed,
        { cout << 10*(row/step) << "%" << endl; //not very accurate for low numbers of timeslots, but it'll do for now
        }
        if (NewFieldorFreq)
        { // at the moment we can't realy handle this?
          cout << "Error new field or frequency detected" << endl;
        }
      }
      ProcessStatistics(); //is there a baseline that should be flagged?
    }
    //===============>>> FrequencyFlagger  <<<===============
  }; // namespace CS1
}; // namespace LOFAR
