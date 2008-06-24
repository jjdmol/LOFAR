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
#include "DataSquasher.h"
#include "MsInfo.h"
#include "RunDetails.h"
#include "DataBuffer.h"


using namespace LOFAR::CS1;
using namespace casa;

//===============>>>  DataSquasher::DataSquasher  <<<===============

DataSquasher::DataSquasher(void)
{
}

//===============>>>  DataSquasher::~DataSquasher  <<<===============

DataSquasher::~DataSquasher(void)
{
}

//===============>>>  DataSquasher::Squash  <<<===============

void DataSquasher::Squash(Matrix<Complex>& oldData, Matrix<Complex>& newData,
                          Matrix<Bool>& oldFlags, Matrix<Bool>& newFlags,
                          Matrix<Float>& newWeights, int itsNumPolarizations,
                          int Start, int Step, int NChan)
{
  int incounter  = 0;
  int outcounter = 0;
  bool flagnew   = true;
  Vector<Complex> values(itsNumPolarizations, 0);
  Vector<Complex> weights(itsNumPolarizations, 0);
  while (incounter < NChan)
  {
    for (int i = 0; i < itsNumPolarizations; i++)
    {
      if (!oldFlags(i, Start + incounter))
      { //existing weight <> 1 is not handled here, maybe sometime in the future?
        values(i) += oldData(i, Start + incounter);
        weights(i) += 1;
        flagnew = false;
      }
    }
    incounter++;
    if ((incounter) % Step == 0)
    {
      if (flagnew)
      {
        newWeights.column(outcounter) = 1.0;
        for (int i = 0; i < itsNumPolarizations; i++)
          newData(i, outcounter)      = values(i)/Step;
        newFlags.column(outcounter)   = true;
      }
      else
      {
        for (int i = 0; i < itsNumPolarizations; i++)
        { values(i) = values(i) / weights(i);
          newWeights(i, outcounter) = abs(weights(i)) / Step;
        }
        newData.column(outcounter)  = values;
        newFlags.column(outcounter) = flagnew;
      }
      values  = 0;
      weights = 0;
      outcounter++;
      flagnew = true;
    }
  }
}

//===============>>>  DataSquasher::ProcessTimeslot  <<<===============

void DataSquasher::ProcessTimeslot(DataBuffer& InData, DataBuffer& OutData,
                                   MsInfo& Info, RunDetails& Details)
{
  //Data.Position is the last filled timeslot, the middle is 1/2 a window behind it
  int pos = (InData.Position - (InData.WindowSize-1)/2) % InData.WindowSize;
  Matrix<Complex> myOldData;
  Matrix<Complex> myNewData;
  Matrix<Bool> myOldFlags;
  Matrix<Bool> myNewFlags;
  Matrix<Float> NewWeights;
  for (int i = 0; i < Info.NumBands; i++)
  {
    for(int j = 0; j < Info.NumAntennae; j++)
    {
      for(int k = j; k < Info.NumAntennae; k++)
      {
        int index = i * Info.NumPairs + Info.BaselineIndex[baseline_t(j, k)];

        InData.Data[index].xyPlane(pos).reference(myOldData);
        OutData.Data[index].xyPlane(pos).reference(myNewData);
        InData.Flags[index].xyPlane(pos).reference(myOldFlags);
        OutData.Flags[index].xyPlane(pos).reference(myNewFlags);
        OutData.Weights[index].xyPlane(pos).reference(NewWeights);
        Squash(myOldData, myNewData, myOldFlags, myNewFlags, NewWeights,
               Info.NumPolarizations, Details.Start, Details.Step, Details.NChan);
        if (Details.Columns)
        {
          InData.ModelData[index].xyPlane(pos).reference(myOldData);
          OutData.ModelData[index].xyPlane(pos).reference(myNewData);
          Squash(myOldData, myNewData, myOldFlags, myNewFlags, NewWeights,
                 Info.NumPolarizations, Details.Start, Details.Step, Details.NChan);
          InData.CorrectedData[index].xyPlane(pos).reference(myOldData);
          OutData.CorrectedData[index].xyPlane(pos).reference(myNewData);
          Squash(myOldData, myNewData, myOldFlags, myNewFlags, NewWeights,
                 Info.NumPolarizations, Details.Start, Details.Step, Details.NChan);
        }
      }
    }
  }
}

//===============>>>  DataSquasher  <<<===============
