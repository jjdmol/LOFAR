//# tModelImageFFT.cc: 
//#
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
//# $Id: tModelImageFFT.cc 20029 2012-05-14 20:50:23Z duscha $


#include <iostream>
#include <BBSKernel/ModelImageFFT.h>

using namespace std;
using namespace casa;

//using std::cout;
//using std::endl;
//using std::complex;
//using std::abs;

void getBaselines(const Table &table, double *baselines[3], 
                  int rowStart=-1, int rowEnd=-1);
vector<double> getMSFrequencies(const Table &table);
void writeUVW(const Table &table, const string &name);

int main(int argc, char **argv)
{
  string imageFilename=argv[1];
  if(argc==3)
    string tableName=argv[2];

  LOFAR::BBS::ModelImageFft modelImage(imageFilename, 20);  // nwplanes=20

  // TODO: Adjust all this to use ModelImageFFT class implementation
  // Change these if necessary to adjust run time
  //

  MeasurementSet ms(tableName, Table::Update);
  double *baselines[3];

  // Get baselines from MS
  getBaselines(ms, baselines);

  exit(0);    // DEBUG
  
  const int nSamples=10000; // Number of data samples
  const int wSize=33; // Number of lookup planes in w projection
  const int nChan=16; // Number of spectral channels

  // Don't change any of these numbers unless you know what you are doing!
  const int gSize=512;          // Size of output grid in pixels
  const double cellSize=40.0;    // Cellsize of output grid in wavelengths
  const int baseline=2000;      // Maximum baseline in meters

  // Initialize the data to be gridded
  vector<double> u(nSamples);
  vector<double> v(nSamples);
  vector<double> w(nSamples);
  vector<std::complex<float> > data(nSamples*nChan);
  vector<std::complex<float> > outdata(nSamples*nChan);

  for (int i=0; i<nSamples; i++)
  {
    u[i]=baseline*double(rand())/double(RAND_MAX)-baseline/2;
    v[i]=baseline*double(rand())/double(RAND_MAX)-baseline/2;
    w[i]=baseline*double(rand())/double(RAND_MAX)-baseline/2;
    for (int chan=0; chan<nChan; chan++)
    {
      data[i*nChan+chan]=1.0;
      outdata[i*nChan+chan]=0.0;
    }
  }
  
  vector<std::complex<float> > grid(gSize*gSize);
  grid.assign(grid.size(), std::complex<float> (0.0));

  // Measure frequency in inverse wavelengths
  std::vector<double> freq(nChan);
  for (int i=0; i<nChan; i++)
  {
    freq[i]=(1.4e9-2.0e5*double(i)/double(nChan))/2.998e8;
  }

  // Initialize convolution function and offsets
//  std::vector<std::complex<float> > C;
  std::vector<std::complex<float> > C;
  int support, overSample;
  std::vector<unsigned int> cOffset;
  // Vectors of grid centers
  std::vector<unsigned int> iu;
  std::vector<unsigned int> iv;
  double wCellSize;
  
  modelImage.initC(nSamples, w, freq, cellSize, baseline, wSize, gSize, support,
      overSample, wCellSize, C);
  modelImage.initCOffset(u, v, w, freq, cellSize, wCellSize, baseline, wSize, gSize,
      support, overSample, cOffset, iu, iv);
  int sSize=2*support+1;

  // Now we can do the timing
  cout << "+++++ Forward processing +++++" << endl;

  clock_t start, finish;
  double time;

  start = clock();
  modelImage.gridKernel(data, support, C, cOffset, iu, iv, grid, gSize);
  finish = clock();
  // Report on timings
  // Report on timings
  time = (double(finish)-double(start))/CLOCKS_PER_SEC;
  cout << "    Time " << time << " (s) " << endl;
  cout << "    Time per visibility spectral sample " << 1e6*time/double(data.size()) << " (us) " << endl;
  cout << "    Time per gridding   " << 1e9*time/(double(data.size())* double((sSize)*(sSize))) << " (ns) " << endl;

  cout << "+++++ Reverse processing +++++" << endl;
  grid.assign(grid.size(), std::complex<float> (1.0));
  
  start = clock();
  modelImage.degridKernel(grid, gSize, support, C, cOffset, iu, iv, outdata);
  finish = clock();
  // Report on timings
  time = (double(finish)-double(start))/CLOCKS_PER_SEC;
  cout << "    Time " << time << " (s) " << endl;
  cout << "    Time per visibility spectral sample " << 1e6*time/double(data.size()) << " (us) " << endl;
  cout << "    Time per degridding " << 1e9*time/(double(data.size())* double((sSize)*(sSize))) << " (ns) " << endl;

  cout << "Done" << endl;

  return 0;
}


// Get baselines from MS from rowStart till rowEnd
//
void getBaselines(const Table &table, double *baselines[3], unsigned int rowStart, unsigned int rowEnd)
{
  // Check that rowStart and rowEnd are within the limits

// Example: 711.309, -357.693, 432.535

  // ROTableColumn UVW: Double Array of Size [ 1 3 ]
  // ROArrayColumn?
}

vector<double> getMSFrequencies(const Table &table)
{
  // Table: SPECTRAL_WINDOW, Column: CHAN_FREQ

}


void writeUVW(const Table &table, const string &name)
{
  // Complex Array of size [ 4 nchannels ]
}