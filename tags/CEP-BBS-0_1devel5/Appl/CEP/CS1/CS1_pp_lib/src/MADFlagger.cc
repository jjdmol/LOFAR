/***************************************************************************
 *   Copyright (C) 2006-8 by ASTRON, Adriaan Renting                       *
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
#include "MADFlagger.h"
#include <casa/Quanta/MVEpoch.h>
#include <casa/Arrays/ArrayMath.h>

#include "MsInfo.h"
#include "RunDetails.h"
#include "DataBuffer.h"
#include "FlaggerStatistics.h"

using namespace LOFAR::CS1;
using namespace casa;

//===============>>>  MADFlagger::MADFlagger  <<<===============
/* initialize some meta data and get the datastorage the right size. */
MADFlagger::MADFlagger():
  NumChannels(0),
  NumPolarizations(0)
{
}

//===============>>>  MADFlagger::~MADFlagger  <<<===============

MADFlagger::~MADFlagger()
{
}

//===============>>> ComplexMedianFlagger2::ComputeThreshold  <<<============
/*Compute Thresholds */
void MADFlagger::ComputeThreshold(const Cube<Complex>& Values,
                                  int TWindowSize, int FWindowSize,
                                  int TimePos, int ChanPos, int PolPos,
                                  float& Z1, float& Z2, Matrix<Float>& Medians)
{
  int temp = 0;
  for (int j = -FWindowSize/2; j <= FWindowSize/2; j++)
  {
    for (int i = -TWindowSize/2; i <= TWindowSize/2; i++)
    {
      temp = ((ChanPos + j < 0 || ChanPos + j >= NumChannels) ? -j : j); //have the channels wrap back upon themselves.
      Medians(i+ TWindowSize/2, j+ FWindowSize/2) = abs(Values(PolPos, ChanPos + temp, TimePos + (i+TWindowSize)%TWindowSize)); //Fill the Matrix.
    }
  }
  Z1 = medianInPlace(Medians);      // Median Vt = Z
  Medians -= Z1;
  Z2 = medianInPlace(abs(Medians)); // Median (Vt - Z) = Z'
  if (isNaN(Z2)) //If there are NaN in the data, then what?
  { Z1 = 0.0;
    Z2 = 0.0;
  }
}

//===============>>> MADFlagger::FlagTimeslot  <<<===============
/* This function inspects each visibility in a cetain baseline-band
and flags on complexe distance, then determines to flag the entire baseline-band
based on the RMS of the points it didn't flag.*/
int MADFlagger::FlagBaselineBand(Matrix<Bool>& Flags,
                                 const Cube<Complex>& Data,
                                 int flagCounter,
                                 double Threshold,
                                 int Position, bool Existing,
                                 int TWindowSize, int FWindowSize)
{
  float Z1         = 0.0;
  float Z2         = 0.0;
  int    flagcount = 0;
  double MAD       = 1.4826;
  Matrix<Float> Medians(TWindowSize, FWindowSize); //A copy of the right size, so we can use medianInPlace
  for (int i = NumChannels-1; i >= 0; i--)
  {
    bool FlagAllPolarizations = false;
    for (int j = NumPolarizations-1; j >= 0; j--)
    { //we need to loop twice, once to determine FlagAllCorrelations
      if (!FlagAllPolarizations /*&& PolarizationsToCheck[j]*/)
      {
        ComputeThreshold(Data, TWindowSize, FWindowSize, Position, i, j, Z1, Z2, Medians);
        FlagAllPolarizations |= (Threshold * Z2 * MAD) < abs(abs(Data(j, i, Position)) - Z1);
      }
    }
    for (int j = NumPolarizations-1; j >= 0; j--)
    { //the second loop we set the flags or calculate RMS
      if (FlagAllPolarizations)
      { Flags(j, i) = true;
        flagcount++;
      }
      else
      { if (!Existing) { Flags(j, i) = false;}
      }
    }
  }
  return flagCounter + flagcount;
}
//===============>>> MADFlagger::FlagBaseline  <<<===============
/* This function iterates over baseline and band and uses FlagBaselineBand() to determine
   for each one if it needs to be flagged. It treats the autocorrelations separately,
   to detect entire malfunctioning telescopes. Finally it writes the flags.
*/
void MADFlagger::ProcessTimeslot(DataBuffer& data,
                                           MsInfo& info,
                                           RunDetails& details,
                                           FlaggerStatistics& stats)
{
  //Data.Position is the last filled timeslot, the middle is 1/2 a window behind it
  int pos = (data.Position + (data.WindowSize+1)/2) % data.WindowSize;
  NumChannels      = info.NumChannels;
  NumPolarizations = info.NumPolarizations;
  int index        = 0;
  Matrix<Bool> flags;
  for (int i = 0; i < info.NumBands; i++)
  {
    for(int j = 0; j < info.NumAntennae; j++)
    {
      for(int k = j; k < info.NumAntennae; k++)
      {
        index    = i * info.NumPairs + info.BaselineIndex[baseline_t(j, k)];
        flags.reference(data.Flags[index].xyPlane(pos));
//        if ((BaselineLengths[BaselineIndex[pairii(j, k)]] < 3000000))//radius of the Earth in meters? WSRT sometimes has fake telescopes at 3854243 m
        stats(i, j, k) = FlagBaselineBand(flags,
                                          data.Data[index],
                                          stats(i,j,k),
                                          details.Treshold,
                                          pos,
                                          details.Existing,
                                          details.TimeWindow,
                                          details.FreqWindow);
      }
    }
  }
}

//===============>>> MADFlagger  <<<===============
