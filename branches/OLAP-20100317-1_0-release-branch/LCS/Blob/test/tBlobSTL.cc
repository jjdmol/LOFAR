//# tBlobSTL.cc: Test program for BlobSTL functions
//#
//# Copyright (C) 2007
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/BlobSTL.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobOBufStream.h>
#include <Blob/BlobIBufStream.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufNull.h>
#include <iostream>
#include <fstream>

using namespace LOFAR;

void doOut (BlobOBuffer& bb)
{
  BlobOStream bs(bb);
  bs.putStart ("test", 1);
  // Write a vector and set into the blob.
  std::vector<double> seq1(3);
  seq1[0] = double(2);
  seq1[1] = double(1e10);
  seq1[2] = double(-3.1);
  bs << seq1;
  std::list<std::string> seq2;
  seq2.push_back ("s1");
  seq2.push_back ("st2");
  bs << seq2;
  // Write maps.
  std::map<std::string, std::list<std::string> > m;
  m["k1"] = seq2;
  seq2.push_back ("str3");
  m["k2"] = seq2;
  bs << m;
  std::map<int, std::vector<int> > m2;
  bs << m2;                     // also test empty map
  bs.putEnd();
}

void doIn (BlobIBuffer& bb)
{
  BlobIStream bs(bb);
  bs.getStart ("test");
  // Read the vector from the blob as a deque and check if it is correct.
  std::deque<double> seq1;
  seq1.push_back (1);          // should be cleared by operator>>
  bs >> seq1;
  ASSERT (seq1.size() == 3);
  ASSERT (seq1[0] == double(2));
  ASSERT (seq1[1] == double(1e10));
  ASSERT (seq1[2] == double(-3.1));
  // Read the list as a set.
  std::set<std::string> seq2;
  seq2.insert ("sss2");         // should be cleared by operator>>
  bs >> seq2;
  ASSERT (seq2.size() == 2);
  ASSERT (seq2.find("s1") != seq2.end());
  ASSERT (seq2.find("st2") != seq2.end());
  ASSERT (seq2.find("sss2") == seq2.end());
  // Read the first map.
  std::map<std::string, std::vector<std::string> > m;
  bs >> m;
  std::map<std::string, std::vector<std::string> >::const_iterator it;
  it = m.find("k1");
  ASSERT (it != m.end());
  ASSERT (it->second.size() == 2);
  ASSERT (it->second[0] == "s1");
  ASSERT (it->second[1] == "st2");
  it = m.find("k2");
  ASSERT (it != m.end());
  ASSERT (it->second.size() == 3);
  ASSERT (it->second[0] == "s1");
  ASSERT (it->second[1] == "st2");
  ASSERT (it->second[2] == "str3");
  it = m.find("k3");
  ASSERT (it == m.end());
  // Read the second map.
  std::map<int, std::list<int> > m2;
  m2[1] = std::list<int>();    // should be cleared by operator>>
  bs >> m2;
  ASSERT (m2.empty());
  bs.getEnd();
}

int main()
{
  try {
    INIT_LOGGER("tBlobSTL");
    {
      {
	// Create the blob in a file.
	std::ofstream os ("tBlobSTL_tmp.dat");
	BlobOBufStream bob(os);
        doOut (bob);
      }
      {
	// Read it back from the file.
	std::ifstream is ("tBlobSTL_tmp.dat");
	BlobIBufStream bib(is);
	doIn (bib);
      }
    }
    {
      // Create the blob in memory.
      BlobString str(BlobStringType(false));
      BlobOBufString bob(str);
      doOut (bob);
      BlobIBufString bib(str);
      doIn (bib);
    }
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}

