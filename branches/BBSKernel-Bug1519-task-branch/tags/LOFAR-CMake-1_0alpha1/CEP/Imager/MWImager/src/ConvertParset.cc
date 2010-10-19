//# ConvertParset.cc: Convert a ParSet file from SAS to cimager format
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <MWImager/ConvertParset.h>
#include <Common/StringUtil.h>
#include <Common/StreamUtil.h>
#include <sstream>
#include <iostream>

using namespace std;

namespace LOFAR {

  void MWImager::convert (ParameterSet& out,
			  const ParameterSet& in,
			  const map<string,string>& old2NewNameMap,
			  const map<string,string>& defaults,
			  const string& prefix)
  {
    // Copy all given keywords to the output.
    // If needed, replace a name.
    for (ParameterSet::const_iterator iter=in.begin();
	 iter != in.end(); ++iter) {
      string name = iter->first;
      map<string,string>::const_iterator loc = old2NewNameMap.find(name);
      if (loc != old2NewNameMap.end()) {
	name = loc->second;
      }
      out.add (prefix + name, iter->second);
    }
    // Now insert defaults if a keyword is not given.
    // The keyword name in the map must be the old name.
    for (map<string,string>::const_iterator iter=defaults.begin();
	 iter != defaults.end(); ++iter) {
      string name = iter->first;
      // Insert default if not part of input paramters.
      if (! in.isDefined (name)) {
	map<string,string>::const_iterator loc = old2NewNameMap.find(name);
	if (loc != old2NewNameMap.end()) {
	  name = loc->second;
	}
	out.add (prefix + name, iter->second);
      }
    }
  }

  ParameterSet MWImager::convertParset (const string& nameIn,
						  const string& nameOut)
  {
    return convertParset (ParameterSet(nameIn), nameOut);
  }

  ParameterSet MWImager::convertParset (const ParameterSet& parset,
						  const string& nameOut)
  {
    map<string,string> emptyMap;
    ParameterSet out;
    ParameterSet in (parset);
    // The output name is the base MS name minus the possible extension
    // and directory.
    string outname = in.getString ("dataset");
    string::size_type pos = outname.rfind ('.');
    if (pos != string::npos) {
      outname = outname.substr (0, pos);
    }
    pos = outname.rfind ('/');
    if (pos != string::npos) {
      outname = outname.substr (pos+1);
    }
    // Convert the gridder keywords.
    {
      ParameterSet grin = in.makeSubset ("Gridder.");
      in.subtractSubset ("Gridder.");
      // Get the gridder type.
      string type = grin.getString ("type");
      out.add ("Cimager.gridder", type);
      grin.remove ("type");
      convert (out, grin, emptyMap, emptyMap, "Cimager.gridder."+type+'.');
    }
    // Convert the solver keywords.
    {
      ParameterSet soin = in.makeSubset ("Solver.");
      in.subtractSubset ("Solver.");
      // Get the solver type (dirty, clean).
      string type = soin.getString ("type");
      soin.remove ("type");
      out.add ("Cimager.solver", type);
      outname += '_' + type;
      convert (out, soin, emptyMap, emptyMap, "Cimager.solver.Clean.");
    }
    // Convert the images keywords.
    {
      ParameterSet imin = in.makeSubset ("Images.");
      in.subtractSubset ("Images.");
      // Combine ra,dec,type into a single string.
      string angle1    = imin.getString ("ra");
      string angle2    = imin.getString ("dec");
      string dirType   = imin.getString ("directionType");
      imin.remove ("ra");
      imin.remove ("dec");
      imin.remove ("directionType");
      vector<string> dirVec(3);
      dirVec[0] = angle1;
      dirVec[1] = angle2;
      dirVec[2] = dirType;
      ostringstream dirVecStr;
      dirVecStr << dirVec;
      // Get nchan, frequencies, cellsizes, and stokes.
      // If needed add unit arcsec to cellsize.
      string nchan     = imin.getString ("nchan", "1");
      string frequency = imin.getString ("frequency");
      vector<string> cellSize = imin.getStringVector ("cellSize");
      vector<string> stokes   = imin.getStringVector ("stokes",
						      vector<string>(1,"I"));
      imin.remove ("nchan");
      imin.remove ("frequency");
      imin.remove ("cellSize");
      imin.remove ("stokes");
      for (unsigned i=0; i<cellSize.size(); ++i) {
	char last = cellSize[i][cellSize[i].size()-1];
	if (last < 'a'  || last > 'z') {
	  cellSize[i] += "arcsec";
	}
      }
      ostringstream cellSizeStr;
      cellSizeStr << cellSize;
      out.add ("Cimager.Images.cellsize", cellSizeStr.str());
      // Form the image names.
      vector<string> names(stokes.size());
      for (unsigned i=0; i<stokes.size(); ++i) {
	stokes[i] = toLower(stokes[i]);
	names[i]  = "image." + stokes[i] + '.' + outname;
      }
      ostringstream namesStr;
      namesStr << names;
      out.add ("Cimager.Images.Names",    namesStr.str());
      // Create individual keywords for freq, nchan, and direction.
      for (unsigned i=0; i<stokes.size(); ++i) {
	string name = "Cimager.Images.image." + stokes[i] + '.' + outname + '.';
	out.add (name+"frequency", frequency);
	out.add (name+"nchan",     nchan);
	out.add (name+"direction", dirVecStr.str());
      }
      // Convert the remaining keywords.
      convert (out, imin, emptyMap, emptyMap, "Cimager.Images.");
    }
    {
      // If needed add unit deg to beamshape.
      vector<string> beam = in.getStringVector ("restore_beam",
						vector<string>(3,"1"));
      in.remove ("restore_beam");
      for (unsigned i=0; i<beam.size(); ++i) {
	char last = beam[i][beam[i].size()-1];
	if (last < 'a'  || last > 'z') {
	  beam[i] += "deg";
	}
      }
      ostringstream beamStr;
      beamStr << beam;
      out.add ("Cimager.restore.beam", beamStr.str());
      map<string,string> defaults;
      defaults["restore"] = "false";
      defaults["ncycles"] = "1";
      map<string,string> old2new;
      old2new["minUV"] = "MinUV";
      old2new["maxUV"] = "MaxUV";
      convert (out, in, old2new, defaults, "Cimager.");
    }
    // Write into a parset file if output name is given.
    if (! nameOut.empty()) {
      out.writeFile (nameOut);
    }
    return out;
  }

}  //# namespace LOFAR
