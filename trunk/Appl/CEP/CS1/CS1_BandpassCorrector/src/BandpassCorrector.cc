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
#include "BandpassCorrector.h"
#include <casa/Quanta/MVEpoch.h>

// Oo nice, hardcoded values. These are the bandpass shape. Source: andre Gunst
double StaticBandpass[] = {
   0.47917056206591,   0.51450263071820,   0.55013957156187,   0.58584868428573,   0.62138865122171,
   0.65651292433973,   0.69097334380148,   0.72452392243909,   0.75692472409382,   0.78794575881674,
   0.81737081468304,   0.84500114455045,   0.87065892660331,   0.89419042002261,   0.91546874161156,
   0.93439619563701,   0.95090609741619,   0.96496404112995,   0.97656857377370,   0.98575124981431,
   0.99257605471542,   0.99713819970126,   0.99956230460231,   1.00000000000000,   0.99862699379184,
   0.99563966036629,   0.99125122245431,   0.98568760608377,   0.97918305760825,   0.97197561825935,
   0.96430255588093,   0.95639585529698,   0.94847786806089,   0.94075721911367,   0.93342506219264,
   0.92665176779672,   0.92058411731185,   0.91534306477258,   0.91102211398211,   0.90768634367826,
   0.90537209750188,   0.90408733910995,   0.90381265630883,   0.90450288199946,   0.90608928445621,
   0.90848226541450,   0.91157449200084,   0.91524437804093,   0.91935982201606,   0.92378210313208,
   0.92836983378122,   0.93298286620382,   0.93748605340990,   0.94175276933707,   0.94566810066759,
   0.94913163249466,   0.95205976184504,   0.95438748660080,   0.95606963223775,   0.95708149459414,
   0.95741889315667,   0.95709764564320,   0.95615249051559,   0.95463549902499,   0.95261403205704,
   0.95016830902312,   0.94738866600390,   0.94437258801850,   0.94122160546060,   0.93803814727657,
   0.93492244330520,   0.93196956537344,   0.92926669134954,   0.92689066856720,   0.92490594309794,
   0.92336290956911,   0.92229672296394,   0.92172659949879,   0.92165561868575,   0.92207102350523,
   0.92294500068830,   0.92423590888613,   0.92588990940517,   0.92784294259746,   0.93002298325291,
   0.93235250072533,   0.93475104425126,   0.93713787113164,   0.93943453520778,   0.94156735536265,
   0.94346968852602,   0.94508393869381,   0.94636324255392,   0.94727278314801,   0.94779069524276,
   0.94790853934551,   0.94763133515851,   0.94697715928785,   0.94597632576900,   0.94467018101252,
   0.94310955670785,   0.94135293468239,   0.93946438637821,   0.93751135621784,   0.93556236248781,
   0.93368469134975,   0.93194215914449,   0.93039301531247,   0.92908805311787,   0.92806898810783,
   0.92736715510567,   0.92700256382656,   0.92698334126653,   0.92730557623643,   0.92795356820590,
   0.92890046941120,   0.93010929639068,   0.93153427515032,   0.93312247341126,   0.93481566419362,
   0.93655235763669,   0.93826993267811,   0.93990679717679,   0.94140450436077,   0.94270975512823,
   0.94377621967076,   0.94456611799130,   0.94505150695426,   0.94521523126914,   0.94505150695426,
   0.94456611799130,   0.94377621967076,   0.94270975512823,   0.94140450436077,   0.93990679717679,
   0.93826993267811,   0.93655235763669,   0.93481566419362,   0.93312247341126,   0.93153427515032,
   0.93010929639068,   0.92890046941120,   0.92795356820590,   0.92730557623643,   0.92698334126653,
   0.92700256382656,   0.92736715510567,   0.92806898810783,   0.92908805311787,   0.93039301531247,
   0.93194215914449,   0.93368469134975,   0.93556236248781,   0.93751135621784,   0.93946438637821,
   0.94135293468239,   0.94310955670785,   0.94467018101252,   0.94597632576900,   0.94697715928785,
   0.94763133515851,   0.94790853934551,   0.94779069524276,   0.94727278314801,   0.94636324255392,
   0.94508393869381,   0.94346968852602,   0.94156735536265,   0.93943453520778,   0.93713787113164,
   0.93475104425126,   0.93235250072533,   0.93002298325291,   0.92784294259746,   0.92588990940517,
   0.92423590888613,   0.92294500068830,   0.92207102350523,   0.92165561868575,   0.92172659949879,
   0.92229672296394,   0.92336290956911,   0.92490594309794,   0.92689066856720,   0.92926669134954,
   0.93196956537344,   0.93492244330520,   0.93803814727657,   0.94122160546060,   0.94437258801850,
   0.94738866600390,   0.95016830902312,   0.95261403205704,   0.95463549902499,   0.95615249051559,
   0.95709764564320,   0.95741889315667,   0.95708149459414,   0.95606963223775,   0.95438748660080,
   0.95205976184504,   0.94913163249466,   0.94566810066759,   0.94175276933707,   0.93748605340990,
   0.93298286620382,   0.92836983378122,   0.92378210313208,   0.91935982201606,   0.91524437804093,
   0.91157449200084,   0.90848226541450,   0.90608928445621,   0.90450288199946,   0.90381265630883,
   0.90408733910995,   0.90537209750188,   0.90768634367826,   0.91102211398211,   0.91534306477258,
   0.92058411731185,   0.92665176779672,   0.93342506219264,   0.94075721911367,   0.94847786806089,
   0.95639585529698,   0.96430255588093,   0.97197561825935,   0.97918305760825,   0.98568760608377,
   0.99125122245431,   0.99563966036629,   0.99862699379184,   1.00000000000000,   0.99956230460231,
   0.99713819970126,   0.99257605471542,   0.98575124981431,   0.97656857377370,   0.96496404112995,
   0.95090609741619,   0.93439619563701,   0.91546874161156,   0.89419042002261,   0.87065892660331,
   0.84500114455045,   0.81737081468304,   0.78794575881674,   0.75692472409382,   0.72452392243909,
   0.69097334380148,   0.65651292433973,   0.62138865122171,   0.58584868428573,   0.55013957156187,
   0.51450263071820};

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
    //===============>>>  BandpassCorrector::BandpassCorrector  <<<===============
    /* initialize some meta data and get the datastorage the right size. */
    BandpassCorrector::BandpassCorrector(MS_File* InputMSfile,
                                         int InputWindowSize)
    {
      MSfile           = InputMSfile;
      WindowSize       = InputWindowSize;
      NumAntennae      = (*MSfile).itsNumAntennae;
      NumPairs         = (*MSfile).itsNumPairs;
      NumBands         = (*MSfile).itsNumBands;
      NumChannels      = (*MSfile).itsNumChannels;
      NumPolarizations = (*MSfile).itsNumPolarizations;
      NumTimeslots     = (*MSfile).itsNumTimeslots;
      AntennaNames     = (*MSfile).itsAntennaNames;

      PairsIndex.resize(NumPairs);

      TimeslotData.resize(NumPairs*NumBands);
      for (int i = 0; i < NumPairs*NumBands; i++)
      { TimeslotData[i].resize(NumPolarizations, NumChannels, WindowSize);
      }

      WriteData.resize(NumPairs*NumBands);
      for (int i = 0; i < NumPairs*NumBands; i++)
      { WriteData[i].resize(NumPolarizations, NumChannels);
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

    //===============>>>  BandpassCorrector::~BandpassCorrector  <<<===============

    BandpassCorrector::~BandpassCorrector()
    {
    }

    //===============>>> BandpassCorrector::ComputeBaselineLengths  <<<===============
    /* compute baseline lengths, and determine the longest one.*/
    void BandpassCorrector::ComputeBaselineLengths()
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

    //===============>>> BandpassCorrector::ProcessBaselineBand  <<<===============
    /*
    */
    void BandpassCorrector::ProcessBaselineBand(Cube<Complex>* Timeslots,
                                                Matrix<Complex>* Data,
                                                int Position, bool fixed)
    {
      for (int i = NumChannels-1; i >= 0; i--)
      {
        for (int j = NumPolarizations-1; j >= 0; j--)
        {
          if (fixed) // we should not do it down here, better is a separate function at the top, but this was easier to implement right now.
          {
            (*Data)(j, i) = (*Timeslots)(j, i, Position) / StaticBandpass[i];
          }
          else
          {
            double MS = 0.0;
            for (int k = 0; k < WindowSize; k++)
            { //This might be faster in some other way ?
              MS += abs((*Timeslots)(j, i, k));
            }
            double RMS = MS / WindowSize;
            (*Data)(j, i) = (*Timeslots)(j, i, Position) / RMS;
          }
        }
      }
    }
    //===============>>> BandpassCorrector::FlagBaseline  <<<===============
    /* This function iterates over baseline and band and uses FlagBaselineBand() to determine
      for each one if it needs to be flagged. It treats the autocorrelations separately,
      to detect entire malfunctioning telescopes. Finally it writes the flags.
    */
    void BandpassCorrector::ProcessTimeslot(TableIterator* write_iter,
                                            TableIterator* data_iter,
                                            Int Position, bool fixed)
    {
      Table         DataTable = (*data_iter).table();
      int           rowcount = DataTable.nrow();
      ROTableVector<Int>     antenna1(DataTable, "ANTENNA1");
      ROTableVector<Int>     antenna2(DataTable, "ANTENNA2");
      ROTableVector<Int>     bandnr  (DataTable, "DATA_DESC_ID");
      ROArrayColumn<Complex> data    (DataTable, "DATA");
      Cube<Complex>          Data(NumPolarizations, NumChannels, rowcount);
      data.getColumn(Data);
      for (int i = 0; i < rowcount; i++)
      {
        int bi    = BaselineIndex[pairii(antenna1(i), antenna2(i))];
        int band  = bandnr(i);
        int index = (band % NumBands) * NumPairs + bi;

        WriteData[index] = Data.xyPlane(i);
      }

      for (int i = 0; i < NumBands; i++)
      {
        for(int j = 0; j < NumAntennae; j++)
        {
          for(int k = j+1; k < NumAntennae; k++)
          {
            int index = i*NumPairs + BaselineIndex[pairii(j, k)];
            if ((BaselineLengths[BaselineIndex[pairii(j, k)]] < 3000000))
            { //we skip bogus telescopes
              ProcessBaselineBand(&(TimeslotData[index]),
                                  &WriteData[index],
                                  Position, fixed);
            }
          }
        }
        for(int j = 0; j < NumAntennae; j++) //write the data
        {
          for(int k = j; k < NumAntennae; k++)
          {
            int index = i*NumPairs + BaselineIndex[pairii(j, k)];
            Matrix<Complex> writedata = WriteData[index];
            (*MSfile).WriteData(write_iter, &writedata);
            (*write_iter)++;
          }
        }
      }
      (*data_iter)++;
    }

    //===============>>> BandpassCorrector::UpdateTimeslotData  <<<===============
    /* This function reads the visibility data for one timeslot and checks if
      a mosaicing mode is active*/
    /* The datastructure Timeslotdata is rather complex because it flattens
      half-filled antenna x antenna x bands matrix into a vector of NumPairs X bands length. */
    bool BandpassCorrector::UpdateTimeslotData(vector<int>* OldFields,
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

    //===============>>> BandpassCorrector::FlagDataOrBaselines  <<<===============
    /* This function iterates over the data per timeslot and uses Flagtimeslot()
      to actually flag datapoints (if flagDatapoints), and entire baselines (if flagRMS)*/
    void BandpassCorrector::CorrectBandpass(bool fixed)
    {
      TableIterator timeslot_iter = (*MSfile).TimeslotIterator();
      TableIterator data_iter     = (*MSfile).TimeslotIterator();
      TableIterator write_iter    = (*MSfile).TimeAntennaIterator();
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
          { ProcessTimeslot(&write_iter, &data_iter, position, fixed);
          }
        }
        if (TimeCounter > WindowSize - 1) //nothing special just falg a timeslot
        { ProcessTimeslot(&write_iter, &data_iter, (TimeCounter + WindowSize/2) % WindowSize, fixed);
        }
        timeslot_iter++;
        if (row++ % step == 0) // to tell the user how much % we have processed,
        { cout << 10*(row/step) << "%" << endl; //not very accurate for low numbers of timeslots, but it'll do for now
        }
        if (timeslot_iter.pastEnd() || NewFieldorFreq)
        { //We still need to flag the last WindowSize/2 timeslots
          for (int position = WindowSize/2 + 1; position < WindowSize; position++)
          { ProcessTimeslot(&write_iter, &data_iter, (TimeCounter + position) % WindowSize, fixed);
          }
          TimeCounter = 0; //reset because we have changed to a new Field or frequency
        }
      }
    }
    //===============>>> BandpassCorrector  <<<===============
  }; // namespace CS1
}; // namespace LOFAR
