//# tIOPerf.cc: Test program for LofarStMan-like IO performance
//# Copyright (C) 2010
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

#include <casa/Containers/Block.h>
#include <casa/OS/Timer.h>
#include <casa/BasicSL/String.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace casa;
using namespace std;

// This program tests the class LofarStMan and related classes.
// The results are written to stdout. The script executing this program,
// compares the results with the reference output file.
//
// It uses the given phase center, station positions, times, and baselines to
// get known UVW coordinates. In this way the UVW calculation can be checked.

uInt nalign (uInt size, uInt alignment)
{
  return (size + alignment-1) / alignment * alignment;
}

void writeData (uInt nseq, uInt nant, uInt nchan, uInt npol,
                uInt alignment)
{
  uInt nrbl = nant*(nant+1)/2;
  // Create and initialize blocks for data, seqnr, and nsample.
  Block<Char> data(nalign(npol*nchan*nrbl*8, alignment), 0);
  Block<Char> seqnr(nalign(4, alignment), 0);
  Block<Char> samples(nalign(nchan*npol*nrbl*2, alignment), 0);
  // Open the file.
  int fd = open ("tIOPerf_tmp.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);
  // Write all data.
  Int64 leng=0;
  Timer timer;
  for (uInt i=0; i<nseq; ++i) {
    leng += write (fd, seqnr.storage(), seqnr.size());
    leng += write (fd, data.storage(), data.size());
    leng += write (fd, samples.storage(), samples.size());
  }
  timer.show ("write  before fsync");
  fsync (fd);
  close (fd);
  timer.show ("write  after fsync ");
  cout << "  wrote " << leng << " bytes" << endl;
}

void writeData2 (uInt nseq, uInt nant, uInt nchan, uInt npol,
                 uInt alignment, uInt nblperfile)
{
  uInt nrbl = nant*(nant+1)/2;
  // Create and initialize blocks for data, seqnr, and nsample.
  Block<Char> data(nalign(npol*nchan*nrbl*8, alignment), 0);
  Block<Char> seqnr(nalign(4, alignment), 0);
  Block<Char> samples(nalign(nchan*npol*nrbl*2, alignment), 0);
  // Open the files.
  uInt nfiles = (nrbl + nblperfile - 1)/ nblperfile;
  Block<int> fds(nfiles);
  for (uInt i=0; i<nfiles; ++i) {
    fds[i] = open (("tIOPerf_tmp.dat"+String::toString(i)).chars(),
                   O_RDWR | O_CREAT | O_TRUNC, 0644);
  }
  // Write all data.
  Int64 leng=0;
  Int64 snr=0;
  Timer timer;
  for (uInt i=0; i<nseq; ++i) {
    for (uInt j=0; j<nfiles; ++j) {
      uInt nb = std::min(nblperfile, nrbl-j*nblperfile);
      for (uInt k=0; k<nb; ++k) {
        *(Int64*)(data.storage()+k*nchan*npol*8) = ++snr;
      }
      leng += write (fds[j], seqnr.storage(), seqnr.size());
      leng += write (fds[j], data.storage(), nb*nchan*npol*8);
      leng += write (fds[j], samples.storage(), nb*nchan*npol*2);
    }
  }
  timer.show ("write2 before fsync");
  for (uInt j=0; j<nfiles; ++j) {
    fsync (fds[j]);
    close (fds[j]);
  }
  timer.show ("write2 after fsync ");
  cout << "  wrote " << leng << " bytes into " << nfiles << " files" << endl;
}

void readSeq (uInt nseq, uInt nant, uInt nchan, uInt npol,
              uInt alignment)
{
  uInt nrbl = nant*(nant+1)/2;
  // Create and initialize blocks for data, seqnr, and nsample.
  Block<Char> data(npol*nchan*nrbl*8, 0);
  Block<Char> seqnr(4, 0);
  Block<Char> samples(nchan*npol*nrbl*2, 0);
  uInt naldata = nalign (data.size(), alignment) - data.size();
  uInt nalseq  = nalign (seqnr.size(), alignment) - seqnr.size();
  uInt nalsamp = nalign (samples.size(), alignment - samples.size());
  Int64 offset = seqnr.size() + nalseq;
  // Open the file.
  int fd = open ("tIOPerf_tmp.dat", O_RDONLY);
  // Read all data.
  Int64 leng = 0;
  Timer timer;
  for (uInt i=0; i<nseq; ++i) {
    lseek (fd, offset, SEEK_SET);
    leng += read (fd, data.storage(), data.size());
    offset += seqnr.size() + nalseq + data.size() + naldata + samples.size() + nalsamp;
  }
  timer.show ("readseq    ");
  cout << " data read " << leng << " bytes; expected "
       << Int64(nseq)*nrbl*npol*nchan*8 << endl;
  close (fd);
}

void readSeq2 (uInt nseq, uInt nant, uInt nchan, uInt npol,
               uInt alignment, uInt nblperfile)
{
  uInt nrbl = nant*(nant+1)/2;
  // Create and initialize blocks for data, seqnr, and nsample.
  Block<Char> data(npol*nchan*nrbl*8, 0);
  Block<Char> seqnr(4, 0);
  Block<Char> samples(nchan*npol*nrbl*2, 0);
  uInt naldata = nalign (data.size(), alignment) - data.size();
  uInt nalseq  = nalign (seqnr.size(), alignment) - seqnr.size();
  uInt nalsamp = nalign (samples.size(), alignment - samples.size());
  // Open the files.
  uInt nfiles = (nrbl + nblperfile - 1)/ nblperfile;
  Block<int> fds(nfiles);
  Block<Int64> offsets(nfiles);
  for (uInt i=0; i<nfiles; ++i) {
    fds[i] = open (("tIOPerf_tmp.dat"+String::toString(i)).chars(),
                   O_RDONLY);
    offsets[i] = seqnr.size() + nalseq;
  }
  // Read all data.
  Int64 leng = 0;
  Timer timer;
  for (uInt i=0; i<nseq; ++i) {
    for (uInt j=0; j<nfiles; ++j) {
      uInt nb = std::min(nblperfile, nrbl-j*nblperfile);
      lseek (fds[j], offsets[j], SEEK_SET);
      leng += read (fds[j], data.storage(), nb*nchan*npol*8);
      offsets[j] += seqnr.size() + nalseq + nb*nchan*npol*10;
    }
  }
  timer.show ("readseq2           ");
  cout << " data read " << leng << " bytes; expected "
       << Int64(nseq)*nrbl*npol*nchan*8 << endl;
  for (uInt j=0; j<nfiles; ++j) {
    close (fds[j]);
  }
}

void readBL (uInt nseq, uInt nant, uInt nchan, uInt npol,
             uInt alignment)
{
  uInt nrbl = nant*(nant+1)/2;
  // Create and initialize blocks for data, seqnr, and nsample.
  Block<Char> data(npol*nchan*8, 0);
  Block<Char> seqnr(4, 0);
  Block<Char> samples(nchan*npol*nrbl*2, 0);
  uInt naldata = nalign (data.size(), alignment) - data.size();
  uInt nalseq  = nalign (seqnr.size(), alignment) - seqnr.size();
  uInt nalsamp = nalign (samples.size(), alignment - samples.size());
  // Open the file.
  int fd = open ("tIOPerf_tmp.dat", O_RDONLY);
  // Read all data.
  Int64 leng = 0;
  Timer timer;
  for (uInt j=0; j<nrbl; ++j) {
    Int64 offset = seqnr.size() + nalseq + j*data.size();
    for (uInt i=0; i<nseq; ++i) {
      lseek (fd, offset, SEEK_SET);
      leng += read (fd, data.storage(), data.size());
      offset += (seqnr.size() + nalseq + naldata + samples.size() + nalsamp
                 + nrbl*data.size());
    }
  }
  timer.show ("readbl     ");
  cout << "  data read " << leng << " bytes; expected "
       << Int64(nseq)*nrbl*npol*nchan*8 << endl;
  close (fd);
}

void readBL2 (uInt nseq, uInt nant, uInt nchan, uInt npol,
              uInt alignment, uInt nblperfile)
{
  uInt nrbl = nant*(nant+1)/2;
  // Create and initialize blocks for data, seqnr, and nsample.
  Block<Char> data(npol*nchan*8, 0);
  Block<Char> seqnr(4, 0);
  Block<Char> samples(nchan*npol*nrbl*2, 0);
  uInt naldata = nalign (data.size(), alignment) - data.size();
  uInt nalseq  = nalign (seqnr.size(), alignment) - seqnr.size();
  uInt nalsamp = nalign (samples.size(), alignment - samples.size());
  // Open the files.
  uInt nfiles = (nrbl + nblperfile - 1)/ nblperfile;
  Block<int> fds(nfiles);
  for (uInt i=0; i<nfiles; ++i) {
    fds[i] = open (("tIOPerf_tmp.dat"+String::toString(i)).chars(),
                   O_RDONLY);
  }
  // Read all data.
  Int64 leng = 0;
  Timer timer;
  for (uInt j=0; j<nrbl; ++j) {
    uInt fdnr = j/nblperfile;
    uInt nb = std::min(nblperfile, nrbl-fdnr*nblperfile);
    Int64 offset = seqnr.size() + nalseq + (j-fdnr*nblperfile)*nchan*npol*8;
    for (uInt i=0; i<nseq; ++i) {
      lseek (fds[fdnr], offset, SEEK_SET);
      leng += read (fds[fdnr], data.storage(), nchan*npol*8);
      Int64 snr = *(Int64*)(data.storage());
      if (snr != 1+j+i*nrbl) {
        cout << "** read snr " << snr << ", expected " << 1+j+i*nrbl << endl;
      }
      offset += (seqnr.size() + nalseq + nb*nchan*npol*10);
    }
  }
  timer.show ("readbl     ");
  cout << "  data read " << leng << " bytes; expected "
       << Int64(nseq)*nrbl*npol*nchan*8 << endl;
  for (uInt j=0; j<nfiles; ++j) {
    close (fds[j]);
  }
}


int main (int argc, char* argv[])
{
  try {
    // Get nseq, nant, nchan, npol from argv.
    uInt nseq=10;
    uInt nant=16;
    uInt nchan=256;
    uInt npol=4;
    uInt align = 512;
    if (argc > 1) {
      istringstream istr(argv[1]);
      istr >> nseq;
    }
    if (argc > 2) {
      istringstream istr(argv[2]);
      istr >> nant;
    }
    if (argc > 3) {
      istringstream istr(argv[3]);
      istr >> nchan;
    }
    if (argc > 4) {
      istringstream istr(argv[4]);
      istr >> npol;
    }
    if (argc > 5) {
      istringstream istr(argv[5]);
      istr >> align;
    }
    uInt nblperfile = (nant+1)/2;
    if (argc > 6) {
      istringstream istr(argv[6]);
      istr >> nblperfile;
    }
    //    writeData (nseq, nant, nchan, npol, align);
    writeData2 (nseq, nant, nchan, npol, align, nblperfile);
    //    readSeq (nseq, nant, nchan, npol, align);
    readSeq2 (nseq, nant, nchan, npol, align, nblperfile);
    //    readBL (nseq, nant, nchan, npol, align);
    readBL2 (nseq, nant, nchan, npol, align, nblperfile);
  } catch (std::exception& x) {
    cout << "Caught an exception: " << x.what() << endl;
    return 1;
  } 
  return 0;                           // exit with success status
}
