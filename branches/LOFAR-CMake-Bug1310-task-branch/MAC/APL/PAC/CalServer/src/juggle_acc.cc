#include <stdio.h>
#include <fstream>

#include <complex>
#include <blitz/array.h>

using namespace blitz;
using namespace std;

int main(int argc, char** argv)
{
  Array<complex<double>, 5> acc;
  Array<complex<double>, 5> newacc;

#if 0
  if (3 == argc && argv[1] && argv[2]) {
    ifstream infile(argv[1]);
    ofstream outfile(argv[2]);

    if (!infile.is_open()) {
      cerr << "Failed to open file for reading: " << argv[0] << endl;
      exit(EXIT_FAILURE);
    }

    if (!outfile.is_open()) {
      cerr << "Failed to open file for writing: " << argv[0] << endl;
      exit(EXIT_FAILURE);
    }      

    infile >> acc;
     
    newacc.resize(acc.extent(thirdDim), acc.extent(fourthDim), acc.extent(fifthDim), acc.extent(firstDim), acc.extent(secondDim));

    for (int i = 0; i < acc.extent(firstDim); i++)
      for (int j = 0; j < acc.extent(secondDim); j++)
	for (int k = 0; k < acc.extent(thirdDim); k++)
	  for (int l = 0; l < acc.extent(fourthDim); l++)
	    for (int m = 0; m < acc.extent(fifthDim); m++)
	      newacc(k,l,m,i,j) = acc(i,j,k,l,m);

    outfile << newacc;
      
  } else {
    cout << "Usage: juggle_acc infile outfile" << endl;
  }

#else

  if (3 == argc && argv[1] && argv[2]) {
    ifstream infile(argv[1]);
    ifstream in2file(argv[2]);

    if (!infile.is_open()) {
      cerr << "Failed to open file for reading: " << argv[0] << endl;
      exit(EXIT_FAILURE);
    }

    if (!in2file.is_open()) {
      cerr << "Failed to open file for writing: " << argv[0] << endl;
      exit(EXIT_FAILURE);
    }      

    infile >> acc;
    in2file >> newacc;
     
    for (int i = 0; i < acc.extent(firstDim); i++)
      for (int j = 0; j < acc.extent(secondDim); j++)
	for (int k = 0; k < acc.extent(thirdDim); k++)
	  for (int l = 0; l < acc.extent(fourthDim); l++)
	    for (int m = 0; m < acc.extent(fifthDim); m++)
	      if (abs(acc(i,j,k,l,m) - newacc(k,l,m,i,j)) > 1e-15) {
		fprintf(stderr, "fout op %d %d %d %d %d\n", i, j, k, l, m);
	      }
      
  } else {
    cout << "Usage: juggle_acc infile outfile" << endl;
  }

#endif

  return 0;
}
