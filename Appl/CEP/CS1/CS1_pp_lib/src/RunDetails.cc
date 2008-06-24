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

#include "RunDetails.h"

using namespace LOFAR::CS1;

//===============>>>  RunDetails::RunDetails  <<<===============

RunDetails::RunDetails()
{
}

//===============>>>  RunDetails::~RunDetails  <<<===============

RunDetails::~RunDetails()
{
}

//===============>>> RunDetails::PrintInfo  <<<===============

void RunDetails::PrintInfo(void)
{
  std::cout << "Fixed:             " << Fixed << std::endl;
  std::cout << "Treshold:          " << Treshold << std::endl;
  std::cout << "MinThreshold:      " << MinThreshold << std::endl;
  std::cout << "MaxThreshold:      " << MaxThreshold << std::endl;
  std::cout << "FreqWindow:        " << FreqWindow << std::endl;
  std::cout << "TimeWindow:        " << TimeWindow << std::endl;
}

//===============>>> RunDetails  <<<===============

