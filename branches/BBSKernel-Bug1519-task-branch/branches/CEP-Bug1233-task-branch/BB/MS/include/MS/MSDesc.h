//# MSDesc.h: Struct to hold the description of an MS
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef BB_MS_MSDESC_H
#define BB_MS_MSDESC_H

// @file
// Struct to hold the description of an MS.
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <string>
#include <vector>
#include <iosfwd>
#include <casa/Arrays/Array.h>

namespace LOFAR {
  //# Forward Declarations
  class BlobOStream;
  class BlobIStream;

  // @ingroup MS
  // @brief Struct to hold the description of an MS.
  // @{

  // This struct holds the basic description of an MS.
  // It is possible to write it into a blob and read it back.
  // It can also be written to an ostream.
  struct MSDesc {
    std::string msPath;    //# path to each part of the MS
    std::string msName;    //# basename of the MS
    int         npart;     //# nr of parts in which the MS is split
    double      ra;        //# Phase center
    double      dec;
    double      startTime;
    double      endTime;
    std::vector<std::string> corrTypes;    //# I,Q,U,V,XX,XY,YX,YY,RR,RL,LR,LL
    std::vector<int>         nchan;        //# nr of channels per band
    std::vector<double>      startFreq;    //# start freq of each band
    std::vector<double>      endFreq;      //# end freq of each band
    std::vector<double>      times;        //# mid time of each sample
    std::vector<double>      exposures;    //# exposure time of each sample
    std::vector<int>         ant1;         //# 1st antenna of each baseline
    std::vector<int>         ant2;         //# 2nd antenna of each baselines
    std::vector<std::string> antNames;     //# maps antennanr to name
    casa::Array<double>      antPos;       //# antenna positions in ITRF
    std::vector<double>      arrayPos;     //# position of the array in ITRF

    void writeDesc (std::ostream& os) const;
  };

  // Write the data into a blob.
  BlobOStream& operator<< (BlobOStream&, const MSDesc&);
  // Read it back from a blob.
  BlobIStream& operator>> (BlobIStream&, MSDesc&);

  // Write it to an ostream.
  std::ostream& operator<< (std::ostream&, const MSDesc&);

  // @}
    
} // end namespace

#endif
