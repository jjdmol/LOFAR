//  BlackBoardDemo.h: An empty simulator  
//
//  Copyright (C) 2000, 2002
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
//
//////////////////////////////////////////////////////////////////////////

#ifndef PSS3_BLACKBOARDDEMO_H
#define PSS3_BLACKBOARDDEMO_H


#include <lofar_config.h>

#include "CEPFrame/Simulator.h"
#include <Common/KeyValueMap.h>


/**
   This is a Simulator class which demonstrates the use of a blackboard control
   architecture in combination with PSS3 calibration software.
*/
/** 
 1. The program uses two database tables for control and as a blackboard. These
    tables have to be created using:
       psql -h dop50 -U postgres <user>

       CREATE TABLE bbsolutions (
       id integer,
       woid integer,
       RA1 double precision, DEC1 double precision, STOKESI1 double precision,
       RA2 double precision, DEC2 double precision, STOKESI2 double precision,
       RA3 double precision, DEC3 double precision, STOKESI3 double precision,
       RA4 double precision, DEC4 double precision, STOKESI4 double precision,
       RA5 double precision, DEC5 double precision, STOKESI5 double precision,
       RA6 double precision, DEC6 double precision, STOKESI6 double precision,
       RA7 double precision, DEC7 double precision, STOKESI7 double precision,
       RA8 double precision, DEC8 double precision, STOKESI8 double precision,
       RA9 double precision, DEC9 double precision, STOKESI9 double precision,
       RA10 double precision, DEC10 double precision, STOKESI10 double precision,
       iteration integer,
       fit double precision,
       mu double precision,
       stddev double precision,
       chi double precision );

       CREATE TABLE bbworkorders (
       woid integer,
       status integer,
       kstype text,
       strategyno integer,
       argsize integer,
       varargblob text,
       p1name text,
       p2name text,
       p3name text,
       solutionid integer );

 2. For each Knowledge Source there must exist a separate measurement set and 
    meqmodel/skymodel tables. See /LOFAR/CEP/CPA/PSS3/MNS/src/parmdb.cc on how
    to create these.

 3. In BlackBoardDemo.cc change 'databaseName' to your own database name.
*/

class BlackBoardDemo: public LOFAR::Simulator
{
public:
  BlackBoardDemo();
  virtual ~BlackBoardDemo();

  // overloaded methods from the Simulator base class
  virtual void define(const LOFAR::KeyValueMap& params = LOFAR::KeyValueMap());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();

  void undefine();

private:
  LOFAR::Step** itsKSSteps;      // Pointer to array of Knowledge Source steps
  LOFAR::Step** itsKSInSteps;    // Pointer to array of input steps to Knowledge Sources
  LOFAR::Step** itsKSOutSteps;   // Pointer to array of output steps to Knowledge Sources

  int itsNumberKS;       // Total number of Knowledge Sources
};


#endif
