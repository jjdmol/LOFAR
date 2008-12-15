//# tParmMerge.cc: test program for merging solvable parms
//#
//# Copyright (C) 2006
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


#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <BBSKernel/ParmData.h>
#include <BBSKernel/Solver.h>
#include <ParmDB/ParmDB.h>
#include <Blob/BlobOBufChar.h>
#include <Blob/BlobIBufChar.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace std;


void readParms (const MeqDomain& domain,
		MeqParmGroup& parmgroup,
		ParmDB::ParmDB& pdb)
{
  // Read all parms for this domain into a single map.
  map<string,ParmDB::ParmValueSet> parmValues;
  vector<string> emptyvec;
  ParmDB::ParmDomain pdomain(domain.startX(), domain.endX(),
			     domain.startY(), domain.endY());
  pdb.getValues (parmValues, emptyvec, pdomain);
  for (MeqParmGroup::iterator iter = parmgroup.begin();
       iter != parmgroup.end();
       ++iter)
  {
    iter->second.fillFunklets (parmValues, domain);
  }
}

void doIt1sd (const ParmDB::ParmDBMeta& pdm)
{
  cout << endl << "tParmMerge with one Prediffer, one domain ..." << endl;
  ParmDB::ParmDB pdb(pdm);
  MeqParmGroup parmgroup;
  MeqPExpr parm1 (MeqParmFunklet::create ("parm1", parmgroup, &pdb));
  MeqPExpr parm2 (MeqParmFunklet::create ("parm2", parmgroup, &pdb));
  MeqPExpr parm3 (MeqParmFunklet::create ("parm2", parmgroup, &pdb));
  MeqDomain workDomain (1,5, 4,10);
  // Read the parm values for this domain.
  readParms (workDomain, parmgroup, pdb);
  vector<MeqDomain> solveDomains(1);
  solveDomains[0] = workDomain;
  // Set the parms to solvable.
  parm1.setSolvable (true);
  parm2.setSolvable (true);
  // Initialize the solvable parms.
  ParmDataInfo parmdata;
  parmdata.set (solveDomains, workDomain);
  const vector<MeqDomain>& localSolveDomains = parmdata.getDomains();
  int nrpert = 0;
  vector<int> nrscids (localSolveDomains.size());
  std::fill (nrscids.begin(), nrscids.end(), 0);
  for (MeqParmGroup::iterator iter = parmgroup.begin();
       iter != parmgroup.end();
       ++iter)
  {
    int nr = iter->second.initDomain (localSolveDomains, nrpert, nrscids);
    if (nr > 0) {
      parmdata.parms().push_back (LOFAR::BBS::ParmData(iter->second.getName(),
					   iter->second.getParmDBSeqNr(),
					   iter->second.getFunklets()));
    }
  }
  parmdata.show (cout);

  Solver solver;
  solver.initSolvableParmData (1, solveDomains, workDomain);
  solver.setSolvableParmData (parmdata, 0);
  solver.show (cout);
}

void doIt4sd (const ParmDB::ParmDBMeta& pdm)
{
  cout << endl << "tParmMerge with one Prediffer, four domains ..." << endl;
  ParmDB::ParmDB pdb(pdm);
  MeqParmGroup parmgroup;
  MeqPExpr parm1 (MeqParmFunklet::create ("parm1d", parmgroup, &pdb));
  MeqPExpr parm2 (MeqParmFunklet::create ("parm2d", parmgroup, &pdb));
  MeqDomain workDomain (1,5, 4,12);
  // Read the parm values for this domain.
  readParms (workDomain, parmgroup, pdb);
  vector<MeqDomain> solveDomains(4);
  solveDomains[0] = MeqDomain(1,5, 4,7);
  solveDomains[1] = MeqDomain(1,5, 7,10);
  solveDomains[2] = MeqDomain(1,5, 10,11);
  solveDomains[3] = MeqDomain(1,5, 11,12);
  // Set the parms to solvable.
  parm1.setSolvable (true);
  parm2.setSolvable (true);
  // Initialize the solvable parms.
  ParmDataInfo parmdata;
  parmdata.set (solveDomains, workDomain);
  const vector<MeqDomain>& localSolveDomains = parmdata.getDomains();
  int nrpert = 0;
  vector<int> nrscids (localSolveDomains.size());
  std::fill (nrscids.begin(), nrscids.end(), 0);
  for (MeqParmGroup::iterator iter = parmgroup.begin();
       iter != parmgroup.end();
       ++iter)
  {
    int nr = iter->second.initDomain (localSolveDomains, nrpert, nrscids);
    if (nr > 0) {
      parmdata.parms().push_back (LOFAR::BBS::ParmData(iter->second.getName(),
					   iter->second.getParmDBSeqNr(),
					   iter->second.getFunklets()));
    }
  }
  parmdata.show (cout);

  Solver solver;
  solver.initSolvableParmData (1, solveDomains, workDomain);
  solver.setSolvableParmData (parmdata, 0);
  solver.show (cout);
}

void doIt4sd2pd (const ParmDB::ParmDBMeta& pdm)
{
  cout << endl << "tParmMerge with two Prediffers, four domains ..." << endl;
  ParmDB::ParmDB pdb(pdm);
  MeqDomain workDomain (1,5, 4,12);
  vector<MeqDomain> solveDomains(4);
  solveDomains[0] = MeqDomain(1,5, 4,7);
  solveDomains[1] = MeqDomain(1,5, 7,10);
  solveDomains[2] = MeqDomain(1,5, 10,11);
  solveDomains[3] = MeqDomain(1,5, 11,12);

  // Have each Prediffer have a different group of parms.
  MeqParmGroup parmgroup1;
  MeqPExpr parm11 (MeqParmFunklet::create ("parm1d", parmgroup1, &pdb));
  MeqPExpr parm12 (MeqParmFunklet::create ("parm2d", parmgroup1, &pdb));
  MeqPExpr parm14 (MeqParmFunklet::create ("parm4d", parmgroup1, &pdb));
  MeqDomain workDomain1 (1,5, 4,10);
  // Read the parm values for this domain.
  readParms (workDomain1, parmgroup1, pdb);
  // Set the parms to solvable.
  parm11.setSolvable (true);
  parm12.setSolvable (true);
  parm14.setSolvable (true);
  // Initialize the solvable parms.
  ParmDataInfo parmdata1;
  parmdata1.set (solveDomains, workDomain1);
  const vector<MeqDomain>& localSolveDomains1 = parmdata1.getDomains();
  int nrpert1 = 0;
  vector<int> nrscids1 (localSolveDomains1.size());
  std::fill (nrscids1.begin(), nrscids1.end(), 0);
  for (MeqParmGroup::iterator iter = parmgroup1.begin();
       iter != parmgroup1.end();
       ++iter)
  {
    int nr = iter->second.initDomain (localSolveDomains1, nrpert1, nrscids1);
    if (nr > 0) {
      parmdata1.parms().push_back (LOFAR::BBS::ParmData(iter->second.getName(),
					    iter->second.getParmDBSeqNr(),
					    iter->second.getFunklets()));
    }
  }
  parmdata1.show (cout);

  MeqParmGroup parmgroup2;
  MeqPExpr parm21 (MeqParmFunklet::create ("parm1d", parmgroup2, &pdb));
  MeqPExpr parm23 (MeqParmFunklet::create ("parm3d", parmgroup2, &pdb));
  MeqPExpr parm24 (MeqParmFunklet::create ("parm4d", parmgroup2, &pdb));
  MeqDomain workDomain2 (1,5, 8,12);
  // Read the parm values for this domain.
  readParms (workDomain2, parmgroup2, pdb);
  // Set the parms to solvable.
  parm21.setSolvable (true);
  parm23.setSolvable (true);
  parm24.setSolvable (true);
  // Initialize the solvable parms.
  ParmDataInfo parmdata2;
  parmdata2.set (solveDomains, workDomain2);
  const vector<MeqDomain>& localSolveDomains2 = parmdata2.getDomains();
  int nrpert2 = 0;
  vector<int> nrscids2 (localSolveDomains2.size());
  std::fill (nrscids2.begin(), nrscids2.end(), 0);
  for (MeqParmGroup::iterator iter = parmgroup2.begin();
       iter != parmgroup2.end();
       ++iter)
  {
    int nr = iter->second.initDomain (localSolveDomains2, nrpert2, nrscids2);
    if (nr > 0) {
      parmdata2.parms().push_back (LOFAR::BBS::ParmData(iter->second.getName(),
					    iter->second.getParmDBSeqNr(),
					    iter->second.getFunklets()));
    }
  }
  parmdata2.show (cout);

  ParmDataInfo bParmData1;
  {
    BlobOBufChar bufo;
    {
      BlobOStream bos(bufo);
      bos << parmdata1;
    }
    BlobIBufChar bufi(bufo.getBuffer(), bufo.size());
    BlobIStream bis(bufi);
    bis >> bParmData1;
  }
  ParmDataInfo bParmData2;
  {
    BlobOBufChar bufo;
    {
      BlobOStream bos(bufo);
      bos << parmdata2;
    }
    BlobIBufChar bufi(bufo.getBuffer(), bufo.size());
    BlobIStream bis(bufi);
    bis >> bParmData2;
  }

  Solver solver;
  solver.initSolvableParmData (2, solveDomains, workDomain);
  solver.setSolvableParmData (bParmData1, 0);
  solver.setSolvableParmData (bParmData2, 1);
  solver.show (cout);
}

int main (int argc, const char* argv[])
{
  try {
    if (argc < 2) {
      cerr << "Run as: tParmMerge parmtable"
	   << endl;
      return 1;
    }

    // Read the info for the ParmTables
    ParmDB::ParmDBMeta pdm("aips", argv[1]);
    // Simple test with 1 solve domain.
    doIt1sd (pdm);
    // Test with four solve domains.
    doIt4sd (pdm);
    // Idem with 2 'prediffers'.
    doIt4sd2pd (pdm);

  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
