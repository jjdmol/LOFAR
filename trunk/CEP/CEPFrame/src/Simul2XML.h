//  Simulator2XML.h: Class to dump a Simul object to XML
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
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_SIMULATOR2XML_H
#define CEPFRAME_SIMULATOR2XML_H

#include "CEPFrame/Simul.h"
#include <Common/lofar_fstream.h>

/** The Simul2XML class outputs a Simul to an XML file */

class Simul2XML
{
public:
  /** Initialize the object with the given root Simul */
  explicit Simul2XML (const Simul& simul);

  ~Simul2XML();

  /** Write XML output to specified file */
  void write (const string &filename);

private:
  /// Helper that recursively outputs XML of all children of the specified Step
  void outputStep (Step* aStep, ofstream& file, int indent=0);
  
  /// Helper that writes out XML of the specified WorkHolder
  void outputWorkHolder (WorkHolder* wh, ofstream& file, int indent=0);

  /// Helper that writes out XML of the specified DataHolder
  void outputDataHolder (DataHolder* dh, ofstream& file, bool input,
			 int indent=0);
  
  /// Root Simul; is not owned by this object
  Simul itsSimul;
  int itsNextID;
};


#endif
