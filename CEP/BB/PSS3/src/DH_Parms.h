//  DH_Parms.h: Example DataHolder
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#ifndef PSS3_DH_PARMS_H
#define PSS3_DH_PARMS_H

#include <Common/lofar_complex.h>

#include <lofar_config.h>

#include "CEPFrame/DH_Postgresql.h"
#include "PSS3/Solution.h"

/**
   This class is an example DataHolder which is only used in the
   Example programs.
*/

class DH_Parms: public LOFAR::DH_Postgresql
{
public:

  explicit DH_Parms (const string& name);

  virtual ~DH_Parms();

  DataHolder* clone() const;

  /// Aloocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  // Data access methods
  const string& getParam1Name();
  float getParam1Value();
  const string& getParam2Name();
  float getParam2Value();
  const string& getParam3Name();
  float getParam3Value();
  int getSourceNo();
  Solution* getSolution();

protected:
  // Definition of the DataPacket type.
  class DataPacket: public DH_Database::DataPacket
  {
  public:
    DataPacket();
    string itsParam1Name;                    // Names and values of three parameters
    float itsParam1Value;
    string itsParam2Name;
    float itsParam2Value;
    string itsParam3Name;
    float itsParam3Value;
    int itsSrcNo;                            // Its source number
    Solution itsSolution;                    // Its solution quality
    long itsID;                              // Unique id
  };

private:

  /// Forbid assignment.
  DH_Parms& operator= (const DH_Parms&);
  DH_Parms(const DH_Parms&);
  DataPacket itsDataPacket;

};

inline const string& DH_Parms::getParam1Name()
{ return itsDataPacket.itsParam1Name;}

inline float DH_Parms::getParam1Value()
{ return itsDataPacket.itsParam1Value; }

inline const string& DH_Parms::getParam2Name()
{ return itsDataPacket.itsParam2Name;}

inline float DH_Parms::getParam2Value()
{ return itsDataPacket.itsParam2Value; }

inline const string& DH_Parms::getParam3Name()
{ return itsDataPacket.itsParam3Name;}

inline float DH_Parms::getParam3Value()
{ return itsDataPacket.itsParam3Value; }

inline int DH_Parms::getSourceNo()
{ return itsDataPacket.itsSrcNo; }

inline Solution* DH_Parms::getSolution()
{ return &itsDataPacket.itsSolution; }

#endif 
