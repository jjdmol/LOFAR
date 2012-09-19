//# tParameterRecord.cc: Test program for class ParameterRecord
//#
//# Copyright (C) 2012
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
#include <Common/ParameterRecord.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

void testScalar()
{
  {
    ParameterValue pv(" { key : abc } ");
    ASSERT (pv.isRecord());
    ParameterRecord rec (pv.getRecord());
    ASSERT (rec.size() == 1);
    ASSERT (rec.getString("key") == "abc");
  }
  {
    ParameterValue pv(" { 'k:ey' : ' abc ' } ");
    ASSERT (pv.isRecord());
    ParameterRecord rec (pv.getRecord());
    ASSERT (rec.size() == 1);
    ASSERT (rec.getString("k:ey") == " abc ");
  }
  {
    ParameterValue pv("{'key':[1,2,3],key2:1}");
    ASSERT (pv.isRecord());
    ParameterRecord rec (pv.getRecord());
    ASSERT (rec.size() == 2);
    ASSERT (rec.getString("key2") == "1");
    ParameterValue prv (rec.get("key"));
    ASSERT (prv.isVector());
    vector<int> v (prv.getIntVector());
    ASSERT (v.size()==3 && v[0]==1 && v[1]==2 && v[2]==3);
  }
}

void testNested()
{
  ParameterValue pv("{key:{k1:{k1a:1,"
                    "k2:[{k1:1,k2a:[{k3:2,k4:3,k:0},'{k3:[4,5]}']},10]}},k2b:3}");
  ASSERT (pv.isRecord());
  ParameterRecord rec (pv.getRecord());
  ASSERT (rec.size() == 2);
  ASSERT (rec.getString("k2b") == "3");
  ParameterValue pvkey (rec.get("key"));
  ASSERT (pvkey.isRecord());
  {
    // Test ParameterSet::getRecord
    ParameterRecord prkey (rec.getRecord("key"));
    ASSERT (prkey.size() == 1);
    ASSERT (prkey.isDefined("k1"));
  }
  ParameterRecord prkey (pvkey.getRecord());
  ParameterValue pvk1 (prkey.get("k1"));
  ASSERT (pvk1.isRecord());
  ParameterRecord reck1 (pvk1.getRecord());
  ASSERT (reck1.size() == 2);
  ASSERT (reck1.getInt("k1a") == 1);
  ParameterValue pvk2 (reck1.get("k2"));
  ASSERT (pvk2.isVector());
  vector<ParameterValue> pv2 (pvk2.getVector());
  ASSERT (pv2.size() == 2);
  ASSERT (pv2[1].getInt() == 10);
  ParameterRecord prk2 (pv2[0].getRecord());
  ASSERT (prk2.size() == 2);
  ASSERT (prk2.getInt("k1") == 1);
  ParameterValue pvk2a (prk2.get("k2a"));
  ASSERT (pvk2a.isVector());
  vector<ParameterValue> pv2a (pvk2a.getVector());
  ASSERT (pv2a.size() == 2);
  ASSERT (pv2a[0].isRecord());
  ParameterRecord prk2a (pv2a[0].getRecord());
  ASSERT (prk2a.size() == 3);
  ASSERT (prk2a.getString("k3") == "2");
  ASSERT (prk2a.getString("k4") == "3");
  ASSERT (prk2a.getString("k") == "0");
  ASSERT (! pv2a[1].isRecord());
  ASSERT (pv2a[1].get() == "'{k3:[4,5]}'");
  ASSERT (pv2a[1].getString() == "{k3:[4,5]}");
}

void testExpand()
{
  {
    ParameterValue pv("[{key0..2}]");
    pv = pv.expand();
    ASSERT (pv.get() == "[{key0},{key1},{key2}]");
  }
  {
    ParameterValue pv("[1*{key:0..2}]");
    pv = pv.expand();
    ASSERT (pv.get() == "[{key:0},{key:1},{key:2}]");
  }
  {
    ParameterValue pv("[{key:[0..2]}]");
    pv = pv.expand();
    ASSERT (pv.get() == "[{key:[0,1,2]}]");
  }
  {
    ParameterValue pv("[{key:[{0..2}3]}]");
    pv = pv.expand();
    ASSERT (pv.get() == "[{key:[03,13,23]}]");
  }
}

void testFind()
{
  ParameterValue pval("{k1.k2.k3:1, k1:{k2.k4:2, k2: {k5:3}}}");
  ParameterRecord rec(pval.getRecord());
  cout << rec << endl;
  ParameterValue pv;
  ASSERT (rec.getRecursive("k1.k2.k3", pv));
  ASSERT (pv.get() == "1");
  ASSERT (rec.getRecursive("k1.k2.k4", pv));
  ASSERT (pv.get() == "2");
  ASSERT (rec.getRecursive("k1.k2.k5", pv));
  ASSERT (pv.get() == "3");
  ASSERT (! rec.getRecursive(".k1.k2.k5", pv));
  ASSERT (! rec.getRecursive("k1.k2.k6", pv));
}

void testFile()
{
  ParameterSet ps("tParameterRecord.in");
  ParameterRecord pr0 (ps.getRecord ("key"));
  ASSERT (pr0.getInt("size") == -1);
  cout << pr0.getString("fileFormat") << endl;
  ASSERT (pr0.getString("fileFormat") == "AIPS++=/CASA");
  ParameterRecord pr1 (ps.getRecord ("key 1"));
  ASSERT (pr1.getInt("size") == 10);
  cout << pr1.getString("fileFormat") << endl;
  ASSERT (pr1.getString("fileFormat") == "AIPS++=/CASA defg\"h klm'n");
}

int main()
{
  try {
    INIT_LOGGER("tParameterRecord");
    testScalar();
    testNested();
    testExpand();
    testFind();
    testFile();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
