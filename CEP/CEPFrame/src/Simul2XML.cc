//  Simulator2XML.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////

#include "BaseSim/Simul2XML.h"
#include <Common/lofar_string.h>

const int INDENT = 1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Simul2XML::Simul2XML (const Simul& simul)
: itsSimul(simul),
  itsNextID(0)
{}

Simul2XML::~Simul2XML()
{}

void Simul2XML::write (const string &filename)
{
  ofstream file(filename.c_str());
  file << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
  file << "<!DOCTYPE basesim SYSTEM \"basesim.dtd\">" << endl;
  file << "<basesim>" << endl;
  outputStep(&itsSimul, file, INDENT);
  file << "</basesim>" << endl;
  file.close();
  cout << "wrote XML output to " << filename << "." << endl;
}

void Simul2XML::outputStep (Step *aStep, ofstream &file, int indent)
{ 
  char *es = new char[indent+1];
  std::fill(es,es+indent,' ');
  es[indent] = 0;
  if (aStep->isSimul()) {
    file << es << "<simul";
  } else {
    file << es << "<step";
  }
  file << " name=\"" << aStep->getName() << aStep->getID() << "\"";
  file << " node=\"node " << aStep->getNode() << "\"";
  file << ">" << endl;
  outputWorkHolder(aStep->getWorker(), file, indent+INDENT);

  if (aStep->isSimul()) {
    Simul* simul = dynamic_cast<Simul*>(aStep);
    if (simul == NULL) {
      cout << "Failed to cast " << aStep->getName() 
	   << " to Simul* in outputStep()" << endl;
      return;
    } else {
      const list<Step*>& steps = simul->getSteps();
      for (list<Step*>::const_iterator iList=steps.begin();
	   iList != steps.end(); iList++) {
	outputStep(*iList, file, indent+INDENT);
      }
    }
  }

  if (aStep->isSimul()) {
    file << es << "</simul>" << endl;
  } else {
    file << es << "</step>" << endl;
  }
  delete [] es;
}

void Simul2XML::outputWorkHolder (WorkHolder *wh, ofstream &file, int indent)
{
  DataHolder *dh;
  char *es = new char[indent+1];
  std::fill(es,es+indent,' ');
  es[indent] = 0;
  file << es << "<workholder";
  file << " class=\"" << wh->getType() << "\">" << endl;
  for (int i=0; i<wh->getInputs(); i++) {
    dh = wh->getInHolder(i);
    outputDataHolder(dh,file,true,indent+INDENT);
  }
  for (int i=0; i<wh->getOutputs(); i++) {
    dh = wh->getOutHolder(i);
    outputDataHolder(dh,file,false,indent+INDENT);
  }
  file << es << "</workholder>" << endl;
  delete [] es;
}

void Simul2XML::outputDataHolder (DataHolder *dh, ofstream &file,
				  bool input, int indent)
{
  int id;
  char *es = new char[indent+1];
  std::fill(es,es+indent,' ');
  es[indent] = 0;
  file << es << "<dataholder";
  file << " name=\"dh";
  if (dh->getTransport().getItsID() == -1) {
    file << itsNextID++ << "\"";
  } else {
    file << dh->getTransport().getItsID() << "\"";
  }
  file << " class=\"" << dh->getType() << "\"";
  DataHolder* tdh = dh->getTransport().getTargetAddr();
  if (tdh != NULL && ( (id = tdh->getTransport().getItsID()) != -1)) {
    file << " connect=\"dh" << id << "\"";
  }
  if (input) {
    file << " type=\"input\"/>" << endl; 
  } else {
    file << " type=\"output\"/>" << endl;
  }
  delete [] es;
}
