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
//
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_SAXPARSER_H
#define CEPFRAME_SAXPARSER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Common/lofar_string.h>
#include "CEPFrame/SAXHandler.h"
#include "CEPFrame/Simul.h"

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
