//#  KeyPrefix.cc: Handle prefixing of ParameterSet keys.
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <APS/KeyPrefix.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace ACC
  {
    namespace APS 
    {

      // Initialization of static variables.
      string KeyPrefix::theirPrefix("");

      KeyPrefix::KeyPrefix(const string& str)
      {
	// String must not be empty and must not contain a dot character.
	if (str.empty() || str.find('.') != string::npos) 
	  THROW (APSException, "Invalid key prefix");
	theirPrefix += str + ".";
      }


      KeyPrefix::~KeyPrefix()
      {
	// First strip off the last character, which must be a dot (.).
	theirPrefix.resize(theirPrefix.size()-1);
	// Then find the now last dot character.
	string::size_type idx = theirPrefix.rfind('.');
	// Strip off everything after the last dot, or clear if no dot found.
	if (idx == string::npos) theirPrefix.clear();
	else theirPrefix = theirPrefix.substr(0, idx+1);
      }


      ostream& operator<<(ostream& os, const KeyPrefix& kp)
      {
	return os << kp.get();
      }


    } // namespace APS

  }  // namespace ACC

} // namespace LOFAR
