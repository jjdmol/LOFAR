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
  myInfo       = info;
  myFile       = msfile;
  myDetails    = details;
  myBandpass   = bandpass;
  myFlagger    = flagger;
  mySquasher   = squasher;
  BandpassData = NULL;
  FlaggerData  = NULL;
  SquasherData = NULL;
  myStatistics = new FlaggerStatistics(*myInfo);
}

//===============>>>  Pipeline::~Pipeline  <<<===============

Pipeline::~Pipeline()
{
  delete myStatistics;
  if (BandpassData)
  {
    delete BandpassData;
    BandpassData = NULL;
  }
  if (FlaggerData)
  {
    delete FlaggerData;
    FlaggerData = NULL;
  }
  if (SquasherData)
  {
    delete SquasherData;
    SquasherData = NULL;
  }
}

//===============>>>  Pipeline::~Pipeline  <<<===============

void Pipeline::MirrorBuffer(DataBuffer& buffer, MsInfo& info)
{
  for (int i = 0; i < info.NumBands * info.NumPairs; i++)
  { buffer.Data[i].xyPlane(buffer.WindowSize - buffer.Position) = buffer.Data[i].xyPlane(buffer.Position);
  }
}

//===============>>> ComplexMedianFlagger::UpdateTimeslotData  <<<===============
void Pipeline::Run(MsInfo* SquashedInfo, bool Columns)
{
  BandpassData = new DataBuffer(myInfo, myDetails->TimeWindow, Columns);
  // Not needed unless Flagger starts altering data, or Bandpass starts altering flags
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
  int           step        = myInfo->NumTimeslots / 100 + 1; //not exact but it'll do
  int           row         = 0;
  while (!read_iter.pastEnd())
  { myFile->UpdateTimeslotData(read_iter, *myInfo, *BandpassData);
    read_iter++;
    if (myBandpass)
    { myBandpass->ProcessTimeslot(*BandpassData, *myInfo, *myDetails);
    }
    if (TimeCounter >= (BandpassData->WindowSize - 1)/2)
    { if (myFlagger)
      { myFlagger->ProcessTimeslot(*BandpassData, *myInfo, *myDetails, *myStatistics);
      }
    }
    else
    { if (TimeCounter > 0)
      { MirrorBuffer(*BandpassData, *myInfo);
      }
    }
    if (TimeCounter >= (BandpassData->WindowSize - 1))
    { if (mySquasher)
      { mySquasher->ProcessTimeslot(*BandpassData, *SquasherData, *myInfo, *myDetails);
      }
      myFile->WriteData(write_iter, *SquashedInfo, *SquasherData);
      write_iter++;
      if (mySquasher) //else SquasherData == FlaggerData
      { SquasherData->Position = (SquasherData->Position + 1) % SquasherData->WindowSize;
      }
    }
    TimeCounter++;
    if (row++ % step == 0) // to tell the user how much % we have processed,
    { cout << (row/step) << "%" << endl; //not very accurate for low numbers of timeslots, but it'll do for now
    }
  }
  for (int i = 1; i <= (BandpassData->WindowSize - 1); i++) //write the last couple of values
  {
    if (myFlagger && i <= (BandpassData->WindowSize - 1)/2)
    { MirrorBuffer(*BandpassData, *myInfo);
      myFlagger->ProcessTimeslot(*BandpassData, *myInfo, *myDetails, *myStatistics);
      BandpassData->Position = ++(BandpassData->Position) % BandpassData->WindowSize;
    }
    if (mySquasher)
    { SquasherData->Position = (SquasherData->Position + 1) % SquasherData->WindowSize;
      mySquasher->ProcessTimeslot(*BandpassData, *SquasherData, *myInfo, *myDetails);
    }
    myFile->WriteData(write_iter, *SquashedInfo, *SquasherData);
    write_iter++;
    SquasherData->Position = (SquasherData->Position + 1) % SquasherData->WindowSize;
    if (row++ % step == 0) // to tell the user how much % we have processed,
    { cout << (row/step) << "%" << endl; //not very accurate for low numbers of timeslots, but it'll do for now
    }
  }
  cout << "Written timeslots: " << TimeCounter << endl;
  myStatistics->PrintStatistics(std::cout);
}

//===============>>> Pipeline  <<<===============

