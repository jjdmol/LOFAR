#include <stdio.h>
#include <complex>
#include <iostream>

#define NUMANTENNAS 96
#define NUMSUBBANDS 512
#define NPOL        2

using namespace std;

int main(int argc, char** argv)
{
  cout << NUMANTENNAS << " x " << NUMANTENNAS << " x " << NUMSUBBANDS << " x " << NPOL << " x " << NPOL << " x " << " [ " << endl;
  for (int ant1 = 0; ant1 < NUMANTENNAS; ant1++) {
    for (int ant2 = 0; ant2 < NUMANTENNAS; ant2++) {
      for (int subband = 0; subband < NUMSUBBANDS; subband++) {
	for (int pol1 = 0; pol1 < NPOL; pol1++) {
	  for (int pol2 = 0; pol2 < NPOL; pol2++) {
	    cout << "(" << pol1 << "," << pol2 << ")  ";
	  }
	}
      }
      cout << endl;
    }
  }
  cout << " ] " << endl;
}
