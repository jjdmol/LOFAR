//# DH_DFTRequest.h: Input DataHolder for the DFT
//#
//# Copyright (C) 2004
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

#ifndef BB_DH_DFTREQUEST_H
#define BB_DH_DFTREQUEST_H


#include <Common/LofarTypes.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{


class DH_DFTRequest: public DataHolder
{
public:
  explicit DH_DFTRequest();

  DH_DFTRequest(const DH_DFTRequest&);

  virtual ~DH_DFTRequest();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get read access to the various data fields.
  // <group>
  double getL() const;
  double getM() const;
  double getN() const;
  const double* getUVW() const;
  double getStartFreq() const;
  double getStepFreq() const;
  int    getNFreq() const;
  double getStartTime() const;
  double getStepTime() const;
  int    getNTime() const;
  const int32* getAnt() const;
  int    getNAnt() const;
  const int32* getAnt1() const;
  const int32* getAnt2() const;
  int    getNBaseline() const;
  // </group>

  // Set the values of LM.
  void setLM (double L, double M);

  // Set the values of various fields.
  // It will create the data block.
  void set (double startFreq, double stepFreq, int nFreq,
	    double startTime, double stepTime, int nTime,
	    int nAnt, int nBaseline);
  /// Give write access to the variable sized data fields.
  // The set function should have been called before.
  // <group>
  double* accessUVW();
  int32*  accessAnt();
  int32*  accessAnt1();
  int32*  accessAnt2();
  // </group>

private:
  /// Forbid assignment.
  DH_DFTRequest& operator= (const DH_DFTRequest&);

  // Fill the pointers to the data in the blob.
  virtual void fillDataPointers();


  double* itsUVW;
  double* itsStartFreq;
  double* itsStepFreq;
  uint32* itsNFreq;
  double* itsStartTime;
  double* itsStepTime;
  uint32* itsNTime;
  double* itsL;
  double* itsM;
  int32*  itsAnt;
  uint32  itsNAnt;
  int32*  itsAnt1;
  int32*  itsAnt2;
  uint32  itsNBaseline;
};


inline const double* DH_DFTRequest::getUVW() const
  { return itsUVW; }

inline double DH_DFTRequest::getStartFreq() const
  { return *itsStartFreq; }
inline double DH_DFTRequest::getStepFreq() const
  { return *itsStepFreq; }
inline int DH_DFTRequest::getNFreq() const
  { return *itsNFreq; }

inline double DH_DFTRequest::getStartTime() const
  { return *itsStartTime; }
inline double DH_DFTRequest::getStepTime() const
  { return *itsStepTime; }
inline int DH_DFTRequest::getNTime() const
  { return *itsNTime; }

inline double DH_DFTRequest::getL() const
  { return *itsL; }
inline double DH_DFTRequest::getM() const
  { return *itsM; }
inline double DH_DFTRequest::getN() const
  { return std::sqrt(*itsL * *itsL + *itsM * *itsM); }

inline const int32* DH_DFTRequest::getAnt() const
  { return itsAnt; }
inline int DH_DFTRequest::getNAnt() const
  { return itsNAnt; }

inline const int32* DH_DFTRequest::getAnt1() const
  { return itsAnt1; }
inline const int32* DH_DFTRequest::getAnt2() const
  { return itsAnt2; }
inline int DH_DFTRequest::getNBaseline() const
  { return itsNBaseline; }

inline double* DH_DFTRequest::accessUVW()
  { return itsUVW; }
inline int32* DH_DFTRequest::accessAnt()
  { return itsAnt; }
inline int32* DH_DFTRequest::accessAnt1()
  { return itsAnt1; }
inline int32* DH_DFTRequest::accessAnt2()
  { return itsAnt2; }

   
}

#endif 
