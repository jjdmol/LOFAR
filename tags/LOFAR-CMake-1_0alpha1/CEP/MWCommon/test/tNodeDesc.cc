//# tNodeDesc.cc: Test program for class NodeDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/NodeDesc.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <ostream>
#include <fstream>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::CEP;

void check (const NodeDesc& node)
{
  ASSERT (node.getName() == "node1");
  ASSERT (node.getFileSys().size() == 2);
  ASSERT (node.getFileSys()[0] == "fs0");
  ASSERT (node.getFileSys()[1] == "fs1");
  ASSERT (node.getMountPoints()[0] == "/fs0/fs1");
  ASSERT (node.getMountPoints()[1] == "/fs1");

  ASSERT (node.findFileSys ("/fs1/abc") == "fs1");
  ASSERT (node.findFileSys ("/fs0/fs1/abc") == "fs0");
  ASSERT (node.findFileSys ("/fs0/abc") == "");
}

void doIt()
{
  NodeDesc node;
  node.setName ("node1");
  node.addFileSys ("fs0", "/auto/fs0/fs1");
  node.addFileSys ("fs1", "/fs1");
  check(node);
  // Write into parset file.
  ofstream fos("tNodeDesc_tmp.fil");
  node.write (fos, "");
  // Read back.
  ParameterSet parset("tNodeDesc_tmp.fil");
  NodeDesc node2(parset);
  check(node2);
  node = node2;
  check(node);
  // Chck that findFileSys handles a single / correctly.
  node.addFileSys ("fs2", "/");
  ASSERT (node.findFileSys ("/fs1/abc") == "fs1");
  ASSERT (node.findFileSys ("/fs0/fs1/abc") == "fs0");
  ASSERT (node.findFileSys ("/fs0/abc") == "fs2");
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
