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

#if 0
    for (int i = 0; i < newacc.extent(firstDim); i++)
      for (int j = 0; j < newacc.extent(secondDim); i++)
	for (int k = 0; k < newacc.extent(thirdDim); j++)
	  for (int l = 0; l < newacc.extent(fourthDim); l++)
	    for (int m = 0; m < newacc.extent(fifthDim); m++)
	      newacc(i,j,k,l,m) = acc(k,i,l,j,m);
#else
    newacc = acc(tensor::k, tensor::l, tensor::m, tensor::i, tensor::j);
#endif

    outfile << newacc;
      
  } else {
    cout << "Usage: juggle_acc infile outfile" << endl;
  }

  return 0;
}
