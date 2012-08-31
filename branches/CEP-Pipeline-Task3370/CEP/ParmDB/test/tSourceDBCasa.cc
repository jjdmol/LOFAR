//# tSourceDBCasa.cc: Program to test the SourceDBCasa class
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
#include <ParmDB/SourceDBCasa.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <tables/Tables/TableRecord.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;
using namespace std;

void testCreate()
{
  {
    ParmDBMeta ptm("casa", "tSourceDBCasa_tmp.tab");
    SourceDB (ptm, true);
  }
  // Test if the created tables exist and are empty.
  Table t1("tSourceDBCasa_tmp.tab");
  ASSERT (t1.nrow() == 0);
  ASSERT (t1.keywordSet().asTable("NAMES").nrow() == 0);
  ASSERT (t1.keywordSet().asTable("DEFAULTVALUES").nrow() == 0);
  ASSERT (t1.keywordSet().asTable("SOURCES").nrow() == 0);
  Table t2("tSourceDBCasa_tmp.tab/DEFAULTVALUES");
  ASSERT (t2.nrow() == 0);
  Table t3("tSourceDBCasa_tmp.tab/NAMES");
  ASSERT (t3.nrow() == 0);
  Table t4("tSourceDBCasa_tmp.tab/SOURCES");
  ASSERT (t4.nrow() == 0);
  ASSERT (t4.keywordSet().asTable("PATCHES").nrow() == 0);
  Table t5("tSourceDBCasa_tmp.tab/SOURCES/PATCHES");
  ASSERT (t5.nrow() == 0);
}

void testPatches()
{
  // Test writing patches.
  SourceDB pdb(ParmDBMeta("casa", "tSourceDBCasa_tmp.tab"), false);
  Table t1("tSourceDBCasa_tmp.tab/SOURCES/PATCHES");
  ASSERT (t1.nrow() == 0);
  ASSERT (pdb.addPatch ("patch1", 1, 2., 1.0, -1.0) == 0);
  ASSERT (pdb.addPatch ("patch2", 2, 3., 1.1, -1.1) == 1);
  ASSERT (pdb.addPatch ("patch3", 1, 5., 1.2, -1.2) == 2);
  ASSERT (pdb.addPatch ("patch4", 1, 4., 1.3, -1.3) == 3);
  ASSERT (t1.nrow() == 4);
  // Try adding an existing patch.
  bool ok = false;
  try {
    pdb.addPatch ("patch1", 11, 12., 11.0, -11.0);
  } catch (std::exception& x) {
    cout << "Expected exception: " << x.what() << endl;
    ok = true;
  }
  ASSERT (ok);
  ASSERT (t1.nrow() == 4);
  // Get the Cat1 patches (in descending order of brightness).
  vector<string> names = pdb.getPatches(1);
  ASSERT (names.size() == 3);
  ASSERT (names[0] == "patch3");
  ASSERT (names[1] == "patch4");
  ASSERT (names[2] == "patch1");
  // Get all patches (in order of category and descending brightness).
  names = pdb.getPatches(-1, "*", 0.1, 100.);
  ASSERT (names.size() == 4);
  ASSERT (names[0] == "patch3");
  ASSERT (names[1] == "patch4");
  ASSERT (names[2] == "patch1");
  ASSERT (names[3] == "patch2");
  ASSERT (pdb.patchExists ("patch1"));
  ASSERT (!pdb.patchExists ("patch10"));
  pdb.checkDuplicates();
}

void testSources()
{
  // Test writing sources.
  SourceDB pdb(ParmDBMeta("casa", "tSourceDBCasa_tmp.tab"), false);
  Table t1("tSourceDBCasa_tmp.tab/SOURCES");
  Table t2("tSourceDBCasa_tmp.tab/SOURCES/PATCHES");
  ASSERT (t1.nrow() == 0);
  ASSERT (t2.nrow() == 4);
  ParmMap defValues;
  defValues.define ("fluxI", ParmValueSet(ParmValue(2)));
  defValues.define ("fluxQ", ParmValueSet(ParmValue(0)));
  defValues.define ("fluxU", ParmValueSet(ParmValue(0)));
  defValues.define ("fluxV", ParmValueSet(ParmValue(0)));
  defValues.define ("SpectralIndex:0", ParmValueSet(ParmValue(1.5)));
  defValues.define ("RotMeas", ParmValueSet(ParmValue(0.75)));
  defValues.define ("PolAng",  ParmValueSet(ParmValue(0.3)));
  defValues.define ("PolFrac", ParmValueSet(ParmValue(0.5)));
  // Add to an existing patch.
  pdb.addSource (SourceInfo("sun", SourceInfo::POINT, "SUN"),
                 "patch1", defValues);
  pdb.addSource (SourceInfo("src1", SourceInfo::POINT, "J2000", 1, 1e9, true),
                 "patch2", defValues,
                 1., -1.);
  ASSERT (t1.nrow() == 2);
  ASSERT (t2.nrow() == 4);
  // Try adding to an unknown patch.
  bool ok = false;
  try {
    pdb.addSource (SourceInfo("src100", SourceInfo::POINT, "J2000"), "patch20",
                   defValues);
  } catch (std::exception& x) {
    cout << "Expected exception: " << x.what() << endl;
    ok = true;
  }
  ASSERT (ok);
  // Try adding an existing source.
  ok = false;
  try {
    pdb.addSource (SourceInfo("src1", SourceInfo::POINT), "patch2", defValues);
  } catch (std::exception& x) {
    cout << "Expected exception: " << x.what() << endl;
    ok = true;
  }
  ASSERT (ok);
  ASSERT (t1.nrow() == 2);
  ASSERT (t2.nrow() == 4);
  // Now add a source as a patch and as a source to that patch.
  pdb.addSource (SourceInfo("src2", SourceInfo::POINT), 3, 2.5, defValues,
                 1.1, -1.1);
  pdb.addSource (SourceInfo("src2a", SourceInfo::DISK), "src2", defValues,
                 1.101, -1.101);
  ASSERT (t1.nrow() == 4);
  ASSERT (t2.nrow() == 5);
  ASSERT (pdb.sourceExists ("sun"));
  ASSERT (!pdb.sourceExists ("moon"));
  ASSERT (pdb.sourceExists ("src1"));
  ASSERT (pdb.sourceExists ("src2"));
  ASSERT (pdb.patchExists ("src2"));
  pdb.checkDuplicates();
  // Get info of src1.
  {
    SourceInfo info1 = pdb.getSource("src1");
    ASSERT (info1.getName() == "src1");
    ASSERT (info1.getType() == SourceInfo::POINT);
    ASSERT (info1.getSpectralIndexNTerms() == 1);
    ASSERT (info1.getSpectralIndexRefFreq() == 1e9);
    ASSERT (info1.getUseRotationMeasure() == true);
  }
  // Now add a duplicate source name (do not check).
  pdb.addSource (SourceInfo("src1", SourceInfo::POINT), "patch2", defValues,
                 1.2, -1.2, false);
  ASSERT (t1.nrow() == 5);
  ASSERT (t2.nrow() == 5);
  ok = false;
  try {
    pdb.checkDuplicates();
  } catch (std::exception& x) {
    cout << "Expected exception: " << x.what() << endl;
    ok = true;
  }
  ASSERT (ok);
  // Now remove src1 (which has duplicates).
  pdb.deleteSources ("src1");
  ASSERT (t1.nrow() == 3);
  ASSERT (t2.nrow() == 5);
  pdb.checkDuplicates();
  // Get some sources.
  SourceInfo info1 = pdb.getSource("sun");
  ASSERT (info1.getName() == "sun");
  ASSERT (info1.getType() == SourceInfo::POINT);
  ASSERT (info1.getRefType() == "SUN");
  ASSERT (info1.getSpectralIndexNTerms() == 0);
  ASSERT (info1.getSpectralIndexRefFreq() == 0.);
  ASSERT (info1.getUseRotationMeasure() == false);
  vector<SourceInfo> vinfo1 = pdb.getSources("s*");
  ASSERT(vinfo1.size() == 3);
  ASSERT(vinfo1[0].getName()=="sun" && vinfo1[0].getType()==SourceInfo::POINT &&
         vinfo1[0].getRefType()=="SUN");
  ASSERT(vinfo1[1].getName()=="src2" && vinfo1[1].getType()==SourceInfo::POINT);
  ASSERT(vinfo1[2].getName()=="src2a" && vinfo1[2].getType()==SourceInfo::DISK);
  vector<SourceInfo> vinfo2 = pdb.getPatchSources("src2");
  ASSERT(vinfo2.size() == 2);
  ASSERT(vinfo2[0].getName()=="src2" && vinfo2[0].getType()==SourceInfo::POINT);
  ASSERT(vinfo2[1].getName()=="src2a" && vinfo2[1].getType()==SourceInfo::DISK);
}

void checkParms()
{
  // Test writing sources.
  ParmDB pdb(ParmDBMeta("casa", "tSourceDBCasa_tmp.tab"), false);
  Table t1("tSourceDBCasa_tmp.tab/DEFAULTVALUES");
  Table t2("tSourceDBCasa_tmp.tab/NAMES");
  Table t3("tSourceDBCasa_tmp.tab");
  ASSERT (t1.nrow() == 28);
  ASSERT (t2.nrow() == 0);
  ASSERT (t3.nrow() == 0);
  ParmMap v;
  pdb.getDefValues (v, "*");
  ASSERT (v.size() == 28);
  ASSERT (v["fluxI:sun"].getFirstParmValue().getValues().data()[0] == 2);
  ASSERT (v["fluxQ:sun"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["fluxU:sun"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["fluxV:sun"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["SpectralIndex:0:sun"].getFirstParmValue().getValues().data()[0] == 1.5);
  ASSERT (v["RotMeas:sun"].getFirstParmValue().getValues().data()[0] == 0.75);
  ASSERT (v["PolAng:sun"].getFirstParmValue().getValues().data()[0] == 0.3);
  ASSERT (v["PolFrac:sun"].getFirstParmValue().getValues().data()[0] == 0.5);
  ASSERT (v["Ra:src2"].getFirstParmValue().getValues().data()[0] == 1.1);
  ASSERT (v["Dec:src2"].getFirstParmValue().getValues().data()[0] == -1.1);
  ASSERT (v["fluxI:src2"].getFirstParmValue().getValues().data()[0] == 2);
  ASSERT (v["fluxQ:src2"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["fluxU:src2"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["fluxV:src2"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["SpectralIndex:0:src2"].getFirstParmValue().getValues().data()[0] == 1.5);
  ASSERT (v["RotMeas:src2"].getFirstParmValue().getValues().data()[0] == 0.75);
  ASSERT (v["PolAng:src2"].getFirstParmValue().getValues().data()[0] == 0.3);
  ASSERT (v["PolFrac:src2"].getFirstParmValue().getValues().data()[0] == 0.5);
  ASSERT (v["Ra:src2a"].getFirstParmValue().getValues().data()[0] == 1.101);
  ASSERT (v["Dec:src2a"].getFirstParmValue().getValues().data()[0] == -1.101);
  ASSERT (v["fluxI:src2a"].getFirstParmValue().getValues().data()[0] == 2);
  ASSERT (v["fluxQ:src2a"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["fluxU:src2a"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["fluxV:src2a"].getFirstParmValue().getValues().data()[0] == 0);
  ASSERT (v["Ra:src2a"].getPertRel() == false);
  ASSERT (v["Dec:src2a"].getPertRel() == false);
  ASSERT (v["fluxI:src2a"].getPertRel() == true);
  ASSERT (v["SpectralIndex:0:src2a"].getFirstParmValue().getValues().data()[0] == 1.5);
  ASSERT (v["RotMeas:src2a"].getFirstParmValue().getValues().data()[0] == 0.75);
  ASSERT (v["PolAng:src2a"].getFirstParmValue().getValues().data()[0] == 0.3);
  ASSERT (v["PolFrac:src2a"].getFirstParmValue().getValues().data()[0] == 0.5);
}

void showData()
{
  SourceDB sdb(ParmDBMeta("", "tSourceDBCasa_tmp.tab"), false);
  sdb.lock();
  sdb.rewind();
  SourceData sdata;
  while (! sdb.atEnd()) {
    sdb.getNextSource (sdata);
    cout << endl;
    cout << "Source name:    " << sdata.getInfo().getName() << endl;
    cout << "Patch name:     " << sdata.getPatchName() << endl;
    cout << "Source type:    " << sdata.getInfo().getType() << endl;
    cout << "Reftype:        " << sdata.getInfo().getRefType() << endl;
    cout << "RA:             " << sdata.getRa() << endl;
    cout << "DEC:            " << sdata.getDec() << endl;
    cout << "I:              " << sdata.getI() << endl;
    cout << "Q:              " << sdata.getQ() << endl;
    cout << "U:              " << sdata.getU() << endl;
    cout << "V:              " << sdata.getV() << endl;
    cout << "Major axis:     " << sdata.getMajorAxis() << endl;
    cout << "Minor axis:     " << sdata.getMinorAxis() << endl;
    cout << "Orientation:    " << sdata.getOrientation() << endl;
    cout << "Spectral index: " << sdata.getInfo().getSpectralIndexNTerms()
         << "  " << sdata.getSpectralIndex() << endl;
    cout << "SpInx RefFreq:  " << sdata.getInfo().getSpectralIndexRefFreq() << endl;
    cout << "Use RM:         " << sdata.getInfo().getUseRotationMeasure() << endl;
    cout << "PolAngle:       " << sdata.getPolarizationAngle() << endl;
    cout << "PolFrac:        " << sdata.getPolarizedFraction() << endl;
    cout << "RM:             " << sdata.getRotationMeasure() << endl;
    cout << "Shapelet I:     " << sdata.getInfo().getShapeletCoeffI() << endl;
    cout << "Shapelet Q:     " << sdata.getInfo().getShapeletCoeffQ() << endl;
    cout << "Shapelet U:     " << sdata.getInfo().getShapeletCoeffU() << endl;
    cout << "Shapelet V:     " << sdata.getInfo().getShapeletCoeffV() << endl;
  }
}

int main()
{
  try {
    INIT_LOGGER("tSourceDBCasa");
    testCreate();
    testPatches();
    testSources();
    checkParms();
    showData();
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
