//  WH_Connection.h: An empty WorkHolder (doing nothing)
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

#ifndef PSS3_WH_CONNECTION_H
#define PSS3_WH_CONNECTION_H

#include <lofar_config.h>

#include <tinyCEP/WorkHolder.h>
#include <PSS3/DH_WorkOrder.h>
#include <PSS3/DH_Solution.h>

namespace LOFAR
{

/**
   This is an empty WorkHolder class. It either has (1 or 2) inputs or outputs of
   a specified type. This WorkHolder is used in a 'stub' step */

class WH_Connection: public LOFAR::WorkHolder
{
public:
  enum connType{input, output};
  enum dhType{WorkOrder, Solution};

  /// Construct the work holder.
  explicit WH_Connection (const string& name, int NInDHs, 
			  int NOutDHs, dhType dh1Type, dhType dh2Type=Solution);

  virtual ~WH_Connection();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const LOFAR::KeyValueMap&);

   /// Make a fresh copy of the WH object.
  virtual WH_Connection* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_Connection (const WH_Connection&);

  /// Forbid assignment.
  WH_Connection& operator= (const WH_Connection&);
  
  connType itsType;
  int      itsNInDHs;
  int      itsNOutDHs;
  dhType   itsDH1Type;
  dhType   itsDH2Type;

};

} // namespace LOFAR

#endif
