//#  Quality.h: contains metrics defining the quality of a solution
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_BBS3_QUALITY_H
#define LOFAR_BBS3_QUALITY_H

// \file Quality.h
// Contains metrics defining the quality of a solution

//# Includes
#include <Common/lofar_iostream.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

// This struct contains metrics defining the quality of a solution.
class Quality {
    
public:
  Quality();

  // Reset all attributes to zero.
  void init();

  void show (ostream& os) const;
  
  bool   itsSolFlag;
  int    itsRank;
  double itsFit;
  double itsMu;
  double itsStddev;
  double itsChi;
};

inline ostream& operator<< (ostream& os, const Quality& qual)
{
  qual.show(os); 
  return os; 
}

// @}

} // namespace LOFAR

#endif

