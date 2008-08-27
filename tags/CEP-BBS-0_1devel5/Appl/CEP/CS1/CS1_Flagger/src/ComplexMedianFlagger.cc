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
#include <tables/Tables.h>
#include <tables/Tables/TableIter.h>
#include "ComplexMedianFlagger.h"
#include <casa/Quanta/MVEpoch.h>

using namespace WSRT; //should be nfra ?
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

//===============>>>  ComplexMedianFlagger::ComplexMedianFlagger  <<<===============
/* initialize some meta data and get the datastorage the right size. */
ComplexMedianFlagger::ComplexMedianFlagger(MS_File* InputMSfile,
                                           int InputWindowSize,
                                           bool UseOnlyXpolarizations,
                                           double InputMinThreshold,
                                           double InputMaxThreshold)
{
  MSfile           = InputMSfile;
  WindowSize       = InputWindowSize;
  MinThreshold     = InputMinThreshold;
  MaxThreshold     = InputMaxThreshold;
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
  PolarizationsToCheck.resize(NumPolarizations);
  DeterminePolarizationsToCheck(UseOnlyXpolarizations);

  TimeslotData.resize(NumPairs*NumBands);
  for (int i = 0; i < NumPairs*NumBands; i++)
  { TimeslotData[i].resize(NumPolarizations, NumChannels, WindowSize);
  }

  int index = 0;
  for (int i = 0; i < NumAntennae; i++)
  { for(int j = i; j < NumAntennae; j++)
    { PairsIndex[index]           = pairii(i, j);
      BaselineIndex[pairii(i, j)] = index++;
    }
  }
  ComputeBaselineLengths();
}

//===============>>>  ComplexMedianFlagger::~ComplexMedianFlagger  <<<===============

ComplexMedianFlagger::~ComplexMedianFlagger()
{
}

//===============>>> ComplexMedianFlagger::DetermineCorrelationsToCheck <<<===============
/* create a list of polarizations we want to check, maybe we only want to to XY, YX */
void ComplexMedianFlagger::DeterminePolarizationsToCheck(bool UseOnlyXpolarizations)
{
  if (UseOnlyXpolarizations)
  {
    bool noCorrError = true;
    for (int i = 0; i < NumPolarizations; i++)
    {
      switch((*MSfile).itsPolarizations[i])
      {
        case None:
        case I:
        case RR:
        case LL:
        case XX:
        case YY:
          if (UseOnlyXpolarizations)
            PolarizationsToCheck[i] = false;
          break;
        case Q:
        case U:
        case V:
        case RL:
        case LR:
        case XY:
        case YX:
          noCorrError = false;
          if (UseOnlyXpolarizations)
            PolarizationsToCheck[i] = true;
          break;
      }
    }
    if (noCorrError)
    {
      cout << "There are no crosspolarizations to flag!";
      exit(1);
    }
  }
  else
  {
    for (int i = 0; i < NumPolarizations; i++)
    {  PolarizationsToCheck[i] = true;
    }
  }
}

//===============>>> ComplexMedianFlagger::ComputeBaselineLengths  <<<===============
/* compute baseline lengths, and determine the longest one.*/
void ComplexMedianFlagger::ComputeBaselineLengths()
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

//===============>>> ComplexMedianFlagger::FlagDataOrBaselines  <<<===============
/*This function outputs the gathered statistics.*/
void ComplexMedianFlagger::ProcessStatistics()
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
    string out = string("IVC") + itoa(i);
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
//===============>>> ComplexMedianFlagger::FlagTimeslot  <<<===============
/* This function inspects each visibility in a cetain baseline-band
and flags on complexe distance, then determines to flag the entire baseline-band
based on the RMS of the points it didn't flag.*/
bool ComplexMedianFlagger::FlagBaselineBand(Matrix<Bool>* Flags,
                                            Cube<Complex>* Timeslots,
                                            vector<double>* rms,
                                            int* flagCounter,
                                            double FlagThreshold,
                                            int Position,
                                            bool flagRMS)
{
  Vector<Float>  Reals(WindowSize);
  Vector<Float>  Imags(WindowSize);
  vector<double> RMS(NumPolarizations);
  vector<int>    RMSCounter(NumPolarizations);
  bool           FlagCompleteRow = true;
  int            flagcount       = 0;
  for (int i = NumChannels-1; i >= 0; i--)
  {
    bool FlagAllPolarizations = false;
    for (int j = NumPolarizations-1; j >= 0; j--)
    { //we need to loop twice, once to determine FlagAllCorrelations
      if (!FlagAllPolarizations && PolarizationsToCheck[j])
      {
        for (int k = 0; k < WindowSize; k++)
        { //This might be faster in some other way ?
          Reals[k] = (*Timeslots)(j, i, k).real();
          Imags[k] = (*Timeslots)(j, i, k).imag();
        }
        float real           = medianInPlace(Reals, false);
        float imag           = medianInPlace(Imags, false);
        FlagAllPolarizations |= FlagThreshold < abs((*Timeslots)(j, i, Position)
                                                   - Complex(real, imag));
      }
    }
    for (int j = NumPolarizations-1; j >= 0; j--)
    { //the second loop we set the flags or calculate RMS
      if (FlagAllPolarizations)
      { (*Flags)(j, i) = true;
        flagcount++;
      }
      else
      {
        FlagCompleteRow = false;
        (*Flags)(j, i)  = false; //could check existing flag ?
        double value    = abs((*Timeslots)(j, i, Position));
        RMS[j]         += value*value;
        (RMSCounter[j])++;
      }
    }
  }
  //these need to be separated out into a different function for clarity
  if (flagcount > 0.9 * NumChannels * NumPolarizations) //more as 90% bad
  {
    FlagCompleteRow = true;
    (*Flags)        = true;
    flagcount       = NumChannels * NumPolarizations;
  }
  if (!FlagCompleteRow && flagRMS) //RMSCounter > 0
  {
    for (int j = NumPolarizations-1; j >= 0; j--)
    {
      (*rms)[j] = sqrt( RMS[j] / RMSCounter[j] );
//      if (PolarizationsToCheck[j] && ((*rms)[j] > 1.5 || (*rms)[j] <= 0.5 * NoiseLevel)) //we think this RMS value is non-physical
      if (PolarizationsToCheck[j] && ((*rms)[j] > 25 || (*rms)[j] <= 0.5 * NoiseLevel)) //we think this RMS value is non-physical
      {
        FlagCompleteRow = true;
        (*Flags)        = true;
        flagcount       = NumChannels * NumPolarizations;
      }
    }
  }
  (*flagCounter) += flagcount;
  return FlagCompleteRow;
}
//===============>>> ComplexMedianFlagger::FlagBaseline  <<<===============
/* This function iterates over baseline and band and uses FlagBaselineBand() to determine
   for each one if it needs to be flagged. It treats the autocorrelations separately,
   to detect entire malfunctioning telescopes. Finally it writes the flags.
*/
void ComplexMedianFlagger::FlagTimeslot(TableIterator* flag_iter,
                                        bool flagDatapoints,
                                        bool flagRMS,
                                        bool ExistingFlags,
                                        Int Position)
{
  Cube<bool>     Flags(NumPolarizations, NumChannels, NumPairs*NumBands, false);
  vector<bool>   FlagCompleteRow(NumPairs*NumBands);
  vector<int>    stats(NumPairs*NumBands);
  for (int i = 0; i < NumBands; i++)
  {
    vector<double>           rms(NumPolarizations);
    vector< vector<double> > AutoCorrRMS(NumPolarizations, NumAntennae);
    for (int j = 0; j < NumPolarizations; j++) // this needs to be written more elegantly
    {
      if ((*MSfile).itsPolarizations[j] == YX)
      { PolarizationsToCheck[j] = false; // We're not checking YX because WSRT writes 0.0 into them
      }
    }
    for(int j = 0; j < NumAntennae; j++)
    { //check auto correlations
      int index = i*NumPairs + BaselineIndex[pairii(j, j)];
      Matrix<Bool> flags = Flags.xyPlane(index);
      FlagCompleteRow[index] = FlagBaselineBand(&flags,
                                                &(TimeslotData[index]),
                                                &rms, &(stats[index]),
                                                3000 * NoiseLevel,
                                                Position, flagRMS);
      for(int k = 0; k < NumPolarizations; k++)
      { AutoCorrRMS[k][j] = rms[k];
      }
    }
    for(int k = 0; k < NumPolarizations; k++) //check on median
    {
      if (PolarizationsToCheck[k])
      {
        double median = medianInPlace(Vector<double>(AutoCorrRMS[k]), false);
        for(int j = 0; j < NumAntennae; j++)
        {
          int index = i*NumPairs + BaselineIndex[pairii(j, j)];
          if (AutoCorrRMS[k][j] < median/10 || AutoCorrRMS[k][j] > median*10)
          { FlagCompleteRow[index] = true;
          }
        }
      }
    }
    for (int j = 0; j < NumPolarizations; j++) // this needs to be written more elegantly
    {
      if ((*MSfile).itsPolarizations[j] == YX)
      { PolarizationsToCheck[j] = true; // We're not checking YX
      }
    }
    // this code coud be separated into different functions
    vector< vector<double> > CrossCorrRMS(NumPolarizations, NumPairs - NumAntennae);
    for(int j = 0; j < NumAntennae; j++)
    {
      for(int k = j+1; k < NumAntennae; k++)
      {
        int index = i*NumPairs + BaselineIndex[pairii(j, k)];
        Matrix<Bool> flags = Flags.xyPlane(index);
        if ((BaselineLengths[BaselineIndex[pairii(j, k)]] < 3000000))//radius of the Earth in meters? WSRT sometimes has fake telescopes at 3854243 m
        { //we skip the non-existent telescopes
          FlagCompleteRow[index] = FlagBaselineBand(&flags,
                                                    &(TimeslotData[index]),
                                                    &rms, &(stats[index]),
                                                    (MinThreshold + (MaxThreshold - MinThreshold)
                                                      * BaselineLengths[BaselineIndex[pairii(j, k)]]
                                                        / MaxBaselineLength
                                                    ) * NoiseLevel,
                                                    Position, flagRMS);
          for(int l = 0; l < NumPolarizations; l++)
          { CrossCorrRMS[l][j*NumAntennae + k - ((j+1)*(j+2))/2] = rms[l]; //a very complex way of indexing, could be simplified?
          }
          if ( ( FlagCompleteRow[i*NumPairs + BaselineIndex[pairii(j, j)]]
               ||FlagCompleteRow[i*NumPairs + BaselineIndex[pairii(k, k)]])
             &&flagRMS && !FlagCompleteRow[index])
          { //We flag it if the autocorrelation is flagged
            FlagCompleteRow[index] = true;
          }
        }
      }
    }
    for(int l = 0; l < NumPolarizations; l++) // check on median
    {
      if (PolarizationsToCheck[l])
      {
        double median = medianInPlace(Vector<double>(CrossCorrRMS[l]), false);
        for(int j = 0; j < NumAntennae; j++)
        {
          for(int k = j+1; k < NumAntennae; k++)
          {
            int index = i*NumPairs + BaselineIndex[pairii(j, k)];
            if (!FlagCompleteRow[index])
            { if(   CrossCorrRMS[l][j*NumAntennae + k - ((j+1)*(j+2))/2] < median/10 //a very complex way of indexing, could be simplified?
                 || CrossCorrRMS[l][j*NumAntennae + k - ((j+1)*(j+2))/2] > median*10)
                 { FlagCompleteRow[index] = true;
              }
            }
          }
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

//===============>>> ComplexMedianFlagger::UpdateTimeslotData  <<<===============
/* This function reads the visibility data for one timeslot and checks if
   a mosaicing mode is active*/
/* The datastructure Timeslotdata is rather complex because it flattens
   half-filled antenna x antenna x bands matrix into a vector of NumPairs X bands length. */
bool ComplexMedianFlagger::UpdateTimeslotData(vector<int>* OldFields,
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
  Cube<Complex>          Data(NumPolarizations, NumChannels, rowcount);

  data.getColumn(Data); //We're not checking Data.nrow() Data.ncolumn(), assuming all data is the same size.
  (*Time) = time(0);//for testing purposes, might be useful in the future

  bool NewField_or_Frequency = false;

  for (int i = 0; i < rowcount; i++)
  {
    int bi    = BaselineIndex[pairii(antenna1(i), antenna2(i))];
    int field = fieldid(i);
    int band  = bandnr(i);
    int index = (band % NumBands) * NumPairs + bi;
    if ((*TimeCounter) > WindowSize - 1)
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

    TimeslotData[index].xyPlane((*TimeCounter) % WindowSize) = Data.xyPlane(i); //TimeslotData is only WindowSize size!
  }
  (*TimeCounter)++; //we want to reset if we detect a gap in DATA_DESC_ID or FIELD. Maybe TIME too???
  return NewField_or_Frequency; //assuming everybody is changing field or frequency at the same time!
}

//===============>>> ComplexMedianFlagger::FlagDataOrBaselines  <<<===============
/* This function iterates over the data per timeslot and uses Flagtimeslot()
   to actually flag datapoints (if flagDatapoints), and entire baselines (if flagRMS)*/
void ComplexMedianFlagger::FlagDataOrBaselines(bool flagDatapoints,
                                               bool flagRMS,
                                               bool ExistingFlags)
{
  TableIterator timeslot_iter = (*MSfile).TimeslotIterator();
  TableIterator flag_iter     = (*MSfile).TimeAntennaIterator();
  int           TimeCounter   = 0;
  double        Time          = 0.0;//for testing purposes
  vector<int>   OldFields(NumPairs);         //to check on multipointing and mosaicing
  vector<int>   OldBands(NumPairs*NumBands); //to check on multifrequency and freq mosaicing

  int           step          = NumTimeslots / 10 + 1; //not exact but it'll do
  int           row           = 0;

  while (!timeslot_iter.pastEnd())
  {
    Table TimeslotTable  = timeslot_iter.table();
    bool  NewFieldorFreq = UpdateTimeslotData(&OldFields,
                                              &OldBands,
                                              &TimeCounter,
                                              &TimeslotTable,
                                              &Time);
    //cout << "Processing: " << MVTime(Time/(24*3600)).string(MVTime::YMD) << endl; //for testing purposes

    if (TimeCounter == WindowSize - 1)
    { //We have filled WindowSize timeslots and need to flag the first WindowSize/2 timeslots
      for (int position = 0; position < WindowSize/2; position++)
      { FlagTimeslot(&flag_iter, flagDatapoints, flagRMS, ExistingFlags, position);
      }
    }
    if (TimeCounter > WindowSize - 1) //nothing special just falg a timeslot
    { FlagTimeslot(&flag_iter, flagDatapoints, flagRMS, ExistingFlags,
                    (TimeCounter + WindowSize/2) % WindowSize);
    }
    timeslot_iter++;
    if (row++ % step == 0) // to tell the user how much % we have processed,
    { cout << 10*(row/step) << "%" << endl; //not very accurate for low numbers of timeslots, but it'll do for now
    }
    if (timeslot_iter.pastEnd() || NewFieldorFreq)
    { //We still need to flag the last WindowSize/2 timeslots
      for (int position = WindowSize/2 + 1; position < WindowSize; position++)
      { FlagTimeslot(&flag_iter, flagDatapoints, flagRMS, ExistingFlags,
                      (TimeCounter + position) % WindowSize);
      }
      TimeCounter = 0; //reset because we have changed to a new Field or frequency
    }
  }
  ProcessStatistics(); //is there a baseline that should be flagged?
}
//===============>>> ComplexMedianFlagger  <<<===============
