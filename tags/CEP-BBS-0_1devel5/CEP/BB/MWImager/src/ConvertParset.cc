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

  ACC::APS::ParameterSet MWImager::convertParset (const string& nameIn,
						  const string& nameOut)
  {
    return convertParset (ACC::APS::ParameterSet(nameIn), nameOut);
  }

  ACC::APS::ParameterSet MWImager::convertParset (const ACC::APS::ParameterSet& in,
						  const string& nameOut)
  {
    ACC::APS::ParameterSet out;
    string outname;
    {
      string datacolumn  = in.getString ("datacolumn");
      string dataset     = in.getString ("dataset");
      string minUV       = in.getString ("minUV");
      string ncycles     = in.getString ("ncycles");
      string restore     = in.getString ("restore");
      vector<double> restoreBeam = in.getDoubleVector ("restore_beam");
      out.add ("Cimager.dataset",    dataset);
      out.add ("Cimager.datacolumn", datacolumn);
      out.add ("Cimager.MinUV",      minUV);
      out.add ("Cimager.ncycles",    ncycles);
      out.add ("Cimager.restore",    restore);
      vector<string> restoreBeamStr (restoreBeam.size());
      for (unsigned i=0; i<restoreBeam.size(); ++i) {
	ostringstream os;
	os << restoreBeam[i];
	restoreBeamStr[i] = os.str() + "deg";
      }
      ostringstream osvec;
      osvec << restoreBeamStr;
      out.add ("Cimager.restore.beam", osvec.str());
      // The output name is the base MS name minus the possible extension.
      outname = dataset;
      string::size_type pos = outname.rfind ('.');
      if (pos != string::npos) {
	outname = outname.substr (0, pos);
      }
      pos = outname.rfind ('/');
      if (pos != string::npos) {
	outname = outname.substr (pos+1);
      }
    }
    {
      ACC::APS::ParameterSet grin = in.makeSubset ("Gridder.");
      string cutoff       = grin.getString ("cutoff");
      string nfacets      = grin.getString ("nfacets");
      string nwplanes     = grin.getString ("nwplanes");
      string oversample   = grin.getString ("oversample");
      string wmax         = grin.getString ("wmax");
      string maxsupport   = grin.getString ("maxsupport");
      string limitsupport = grin.getString ("limitsupport", "0");
      string type         = grin.getString ("type");
      string name = "Cimager.gridder." + type;
      out.add ("Cimager.gridder",      type);
      out.add (name + ".wmax",         wmax);
      out.add (name + ".nwplanes",     nwplanes);
      out.add (name + ".oversample",   oversample);
      out.add (name + ".cutoff",       cutoff);
      out.add (name + ".maxsupport",   maxsupport);
      out.add (name + ".limitsupport", limitsupport);
    }
    {
      ACC::APS::ParameterSet soin = in.makeSubset ("Solver.");
      string algorithm = soin.getString ("algorithm");
      string gain      = soin.getString ("gain");
      string niter     = soin.getString ("niter");
      string type      = soin.getString ("type");
      string verbose   = soin.getString ("verbose");
      string scales    = soin.getString ("scales");
      out.add ("Cimager.solver", type);
      out.add ("Cimager.solver.Clean.algorithm", algorithm);
      out.add ("Cimager.solver.Clean.niter",     niter);
      out.add ("Cimager.solver.Clean.gain",      gain);
      out.add ("Cimager.solver.Clean.verbose",   verbose);
      out.add ("Cimager.solver.Clean.scales",    scales);
      outname += '_' + type;
    }
    {
      ACC::APS::ParameterSet imin = in.makeSubset ("Images.");
      string angle1    = imin.getString ("ra");
      string angle2    = imin.getString ("dec");
      string dirType   = imin.getString ("directionType");
      string nchan     = imin.getString ("nchan", "1");
      string shape     = imin.getString ("shape");
      string frequency = imin.getString ("frequency");
      vector<string> stokes   = imin.getStringVector ("stokes");
      vector<int32>  cellSize = imin.getInt32Vector  ("cellSize");
      vector<string> cellSizeVec (cellSize.size());
      for (unsigned i=0; i<cellSize.size(); ++i) {
	ostringstream os;
	os << cellSize[i] << "arcsec";
	cellSizeVec[i] = os.str();
      }
      ostringstream cellSizeStr;
      cellSizeStr << cellSizeVec;
      vector<string> names(stokes.size());
      for (unsigned i=0; i<stokes.size(); ++i) {
	stokes[i] = toLower(stokes[i]);
	names[i]  = "image." + stokes[i] + '.' + outname;
      }
      ostringstream namesStr;
      namesStr << names;
      out.add ("Cimager.Images.Names",    namesStr.str());
      out.add ("Cimager.Images.shape",    shape);
      out.add ("Cimager.Images.cellsize", cellSizeStr.str());
      vector<string> dirVec(3);
      dirVec[0] = angle1;
      dirVec[1] = angle2;
      dirVec[2] = dirType;
      ostringstream dirVecStr;
      dirVecStr << dirVec;
      for (unsigned i=0; i<stokes.size(); ++i) {
	string name = "Cimager.Images.image." + stokes[i] + '.' + outname + '.';
	out.add (name+"frequency", frequency);
	out.add (name+"nchan",     nchan);
	out.add (name+"direction", dirVecStr.str());
      }
    }
    // Write into a parset file if output name is given.
    if (! nameOut.empty()) {
      out.writeFile (nameOut);
    }
    return out;
  }

}  //# namespace LOFAR
