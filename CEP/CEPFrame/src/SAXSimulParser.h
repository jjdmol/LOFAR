//  SAXParser.h:
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
//  $Log$
//  Revision 1.2  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.1  2002/03/15 13:28:08  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_SAXPARSER_H
#define BASESIM_SAXPARSER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Common/lofar_string.h>
#include "BaseSim/SAXHandler.h"
#include "BaseSim/Simul.h"

#if !defined(HAVE_XERCES)
class XMLCh;
class SAX2XMLReader;
class SAXHandler;
#endif

/** This is a wrapper around the SAX2XMLReader class, to make parsing
    an XML file for building a LOFAR simulation easy.
*/

class SAXSimulParser
{
public:
  
  SAXSimulParser (const string& fileName);
  SAXSimulParser (const string& fileName, const string& rootSimul);
  ~SAXSimulParser();

  Simul parseSimul();

private:
  void init (const char* fileName, const char* rootSimul);

  XMLCh*           file;
  SAX2XMLReader*   parser;
  SAXHandler*      handler;
};


#endif
