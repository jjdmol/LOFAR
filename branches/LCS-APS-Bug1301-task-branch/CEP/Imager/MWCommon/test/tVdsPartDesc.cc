//# tVdsPartDesc.cc: Test program for class VdsPartDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/VdsPartDesc.h>
#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <ostream>
#include <fstream>

using namespace LOFAR;
using namespace LOFAR::CEP;
using namespace std;

void check (const VdsPartDesc& vds, uint nTimes=0)
{
  ASSERT (vds.getName() == "/usr/local/xyx");
  ASSERT (vds.getFileName() == "/usr/local/abcd");
  ASSERT (vds.getFileSys() == "node1:/usr");
  ASSERT (vds.getStartTime() == 0.5);
  ASSERT (vds.getEndTime() == 1);
  ASSERT (vds.getStepTime() == 0.25);
  ASSERT (vds.getStartTimes().size() == nTimes);
  ASSERT (vds.getEndTimes().size() == nTimes);
  if (nTimes > 0) {
    ASSERT (vds.getStartTimes()[0] == 1);
    ASSERT (vds.getStartTimes()[nTimes-1] == 1);
    ASSERT (vds.getEndTimes()[0] == 2);
    ASSERT (vds.getEndTimes()[nTimes-1] == 2);
  }
  ASSERT (vds.getNChan().size() == 2);
  ASSERT (vds.getNChan()[0] == 2);
  ASSERT (vds.getNChan()[1] == 3);
  ASSERT (vds.getStartFreqs().size() == 5);
  ASSERT (vds.getStartFreqs()[0] == 20);
  ASSERT (vds.getStartFreqs()[1] == 60);
  ASSERT (vds.getStartFreqs()[2] == 123456789);
  ASSERT (vds.getStartFreqs()[3] == 123456790);
  ASSERT (vds.getStartFreqs()[4] == 123456791);
  ASSERT (vds.getEndFreqs().size() == 5);
  ASSERT (vds.getEndFreqs()[0] == 60);
  ASSERT (vds.getEndFreqs()[1] == 100);
  ASSERT (vds.getEndFreqs()[2] == 123456790);
  ASSERT (vds.getEndFreqs()[3] == 123456791);
  ASSERT (vds.getEndFreqs()[4] == 123456792);
  ASSERT (vds.getParms().size() == 1);
  ASSERT (vds.getParms().getString("key1") == "value1");
}

void doIt()
{
  VdsPartDesc vds;
  vds.setName ("/usr/local/abcd", "node1:/usr");
  ASSERT (vds.getFileName().empty());
  vds.setFileName ("/usr/local/abcd");
  vds.changeBaseName ("xyx");
  vds.setTimes (0.5, 1, 0.25);
  vds.addBand (2, 20, 100);
  vds.addBand (3, 123456789, 123456792);
  vds.addParm ("key1", "value1");
  check(vds);
  // Write into parset file.
  ofstream fos("tVdsPartDesc_tmp.fil");
  vds.write (fos, "");
  // Read back.
  ParameterSet parset("tVdsPartDesc_tmp.fil");
  VdsPartDesc vds2(parset);
  check(vds2);
  vds = vds2;
  check(vds);
  // Check writing/reading from/to blob.
  BlobString bstr;
  BlobOBufString bobs(bstr);
  BlobOStream bos(bobs);
  bos << vds;
  BlobIBufString bibs(bstr);
  BlobIStream bis(bibs);
  VdsPartDesc vdsb;
  bis >> vdsb;
  check (vdsb);
  // Add some times and check again.
  vdsb.setTimes (0.5, 1, 0.25, vector<double>(10,1), vector<double>(10,2));
  check (vdsb, 10);
  // Check if times are written as well.
  ofstream fos2("tVdsPartDesc_tmp.fil2");
  vdsb.write (fos2, "");
  VdsPartDesc vdsb2(ParameterSet("tVdsPartDesc_tmp.fil2"));
  check(vdsb2, 10);
  vdsb.clearParms();
  ASSERT (vdsb.getParms().size() == 0);
}

int main()
{
  try {
    doIt();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
