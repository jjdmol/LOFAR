//#  WH_LopesDataReader.h:
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
//#

#ifndef STATIONSIM_WH_LOPESDATAREADER_H
#define STATIONSIM_WH_LOPESDATAREADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleC.h>

#include <dirent.h>
#include <string>
#include "vector.h"
#include <sys/stat.h>
#include <unistd.h>

typedef short int buftype;

class WH_LopesDataReader: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_LopesDataReader (const string& name, unsigned int nrcu, const string& dirName);

  virtual ~WH_LopesDataReader();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_LopesDataReader* make (const string& name) const;

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// There are no input DataHolders.
  virtual DataHolder* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_SampleC* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_LopesDataReader (const WH_LopesDataReader&);

  /// Forbid assignment.
  WH_LopesDataReader& operator= (const WH_LopesDataReader&);
  
  // Get the directory content
  void getDirEntries (const string& dirname, vector<dirent*>& entries);
  void readLopesHeader ();

  /// Pointer to the array of output DataHolders.
  DH_SampleC   itsOutHolder;
  string       itsDirName;
  ifstream*    itsCurrentFile;
  int          itsDirPointer;
  unsigned int itsBlockPointer;
  unsigned int itsNrcu;
  unsigned int itsLength;
  buftype**    itsBuffer;
  vector<dirent*> entries;
  int          itsCount;
};


#endif
