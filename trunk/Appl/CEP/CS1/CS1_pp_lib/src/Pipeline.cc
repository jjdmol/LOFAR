/***************************************************************************
 *   Copyright (C) 2007-8 by ASTRON, Adriaan Renting                       *
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
#include <iostream>

#include "Pipeline.h"
#include "MsInfo.h"
#include "MsFile.h"
#include "RunDetails.h"
#include "BandpassCorrector.h"
#include "Flagger.h"
#include "DataSquasher.h"
#include "DataBuffer.h"
#include "FlaggerStatistics.h"

using namespace LOFAR::CS1;
using namespace casa;

//===============>>>  Pipeline::Pipeline  <<<===============

Pipeline::Pipeline(MsInfo* info, MsFile* msfile, RunDetails* details,
                   BandpassCorrector* bandpass, Flagger* flagger, DataSquasher* squasher)

{
  myInfo     = info;
  myFile     = msfile;
  myDetails  = details;
  myBandpass = bandpass;
  myFlagger  = flagger;
  mySquasher = squasher;
  myStatistics = new FlaggerStatistics(*myInfo);
}

//===============>>>  Pipeline::~Pipeline  <<<===============

Pipeline::~Pipeline()
{
}

//===============>>>  Pipeline::~Pipeline  <<<===============

void Pipeline::initBuffer(DataBuffer& buffer, MsInfo& info)
{
  for (int i = 0; i < info.NumBands * info.NumPairs; i++)
  { buffer.Data[i].xyPlane(buffer.WindowSize -1 - buffer.Position) = buffer.Data[i].xyPlane(buffer.Position);
  }
}

//===============>>> ComplexMedianFlagger::UpdateTimeslotData  <<<===============
void Pipeline::Run(MsInfo* SquashedInfo, bool Columns)
{
  BandpassData = new DataBuffer(myInfo, myDetails->TimeWindow, Columns);
  //  if (myFlagger && myBandpass)
  //  { FlaggerData = new DataBuffer(info, myDetails->TimeWindow);
  //  }
  //  else
  //  { FlaggerData = BandpassData;
  //  }
  FlaggerData = BandpassData;
  if (mySquasher)
  { SquasherData = new DataBuffer(SquashedInfo, myDetails->TimeWindow, Columns);
  }
  else
  { SquasherData = FlaggerData;
  }

  TableIterator read_iter   = (*myFile).ReadIterator();
  TableIterator write_iter  = (*myFile).WriteIterator();
  int           TimeCounter = 0;
  int           step        = myInfo->NumTimeslots / 10 + 1; //not exact but it'll do
  int           row         = 0;
  while (!read_iter.pastEnd())
  { cout << "reading data" << endl;
    myFile->UpdateTimeslotData(read_iter, *myInfo, *BandpassData);
    read_iter++;
    if (myBandpass)
    { cout << "Running Bandpass correction" << endl;
      myBandpass->ProcessTimeslot(*BandpassData, *myInfo, *myDetails);
    }
    if (TimeCounter >= (BandpassData->WindowSize - 1)/2)
    {
      if (myFlagger)
      { cout << "Running flagger" << endl;
        myFlagger->ProcessTimeslot(*BandpassData, *myInfo, *myDetails, *myStatistics);
      }
      if (mySquasher)
      { cout << "Running Data reduction" << endl;
        SquasherData->Position = (SquasherData->Position + 1) % SquasherData->WindowSize;
        cout << SquasherData->Position << endl;
        mySquasher->ProcessTimeslot(*BandpassData, *SquasherData, *myInfo, *myDetails);
      }
      cout << "writing data" << endl;
      myFile->WriteData(write_iter, *myInfo, *SquasherData);
      write_iter++;
    }
    else
    {
      initBuffer(*BandpassData, *myInfo);
    }
    TimeCounter++;
    if (row++ % step == 0) // to tell the user how much % we have processed,
    { cout << 10*(row/step) << "%" << endl; //not very accurate for low numbers of timeslots, but it'll do for now
    }
  }
  myStatistics->PrintStatistics(std::cout);
}

//===============>>> Pipeline  <<<===============

