//# ConvertParset.cc: Convert a ParSet file from SAS to cimager format
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
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
    // and directory unless an image name is given.
    string inname  = in.getString ("dataset");
    string outname = in.getString ("Images.name", inname);
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
      string type    = grin.getString ("type");
      string padding = grin.getString ("padding", string());
      out.add ("Cimager.gridder", type);
      grin.remove ("type");
      grin.remove ("padding");
      convert (out, grin, emptyMap, emptyMap, "Cimager.gridder."+type+'.');
      if (! padding.empty()) {
        out.add ("Cimager.gridder.padding", padding);
      }
    }
    // Convert the solver keywords.
    {
      ParameterSet soin = in.makeSubset ("Solver.");
      in.subtractSubset ("Solver.");
      // Get the solver type (dirty, clean).
      string type = soin.getString ("type", "dirty");
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
      // Get it as given; otherwise use field direction from MS.
      string angle1    = imin.getString ("ra", string());
      string angle2    = imin.getString ("dec", string());
      string dirType   = imin.getString ("directionType", "J2000");
      if (angle1.empty() || angle2.empty()) {
        angle1 = in.getString ("msDirRa", string());
        angle2 = in.getString ("msDirDec", string());
        dirType = in.getString ("msDirType", string());
      }
      string nfacets   = imin.getString ("nfacets", string());
      string facetstep = imin.getString ("facetstep", string());
      imin.remove ("name");
      imin.remove ("ra");
      imin.remove ("dec");
      imin.remove ("directionType");
      imin.remove ("nfacets");
      imin.remove ("facetstep");
      in.remove ("msDirRa");
      in.remove ("msDirDec");
      in.remove ("msDirType");
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
      out.add ("Cimager.Images.Names", "image." + outname);
      string prefix = "Cimager.Images.image." + outname;
      out.add (prefix + ".frequency", frequency);
      out.add (prefix + ".nchan", nchan);
      out.add (prefix + ".direction", dirVecStr.str());
      if (! nfacets.empty()) {
        out.add (prefix + ".nfacets", nfacets);
        if (! facetstep.empty()) {
          out.add (prefix + ".facetstep", facetstep);
        }
      }
      // Form the stokes string (separated by blanks).
      string stokesStr("'");
      for (unsigned i=0; i<stokes.size(); ++i) {
	stokes[i] = toUpper(stokes[i]);
        if (i != 0) {
          stokesStr += ' ';
        }
        stokesStr += stokes[i];
      }
      out.add (prefix + ".polarisation", stokesStr+"'");
      // Convert the remaining keywords.
      convert (out, imin, emptyMap, emptyMap, "Cimager.Images.");
    }
        //      float padding = imin.getFloat ("padding", 1);
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
