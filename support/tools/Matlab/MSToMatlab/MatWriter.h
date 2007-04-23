#ifndef MatWriter_H
#define MatWriter_H

#include "mat.h"
#include <casa/Arrays.h>
#include <casa/BasicMath/Math.h>
#include <casa/aipstype.h>
#include <casa/complex.h>

using namespace std;
using namespace casa;

class MatWriter{
	public:
		MatWriter(int dim1, int dim2, int dim3);
		~MatWriter();
		int openFile(const string& fileName);
		int writeCube(const Cube<complex<float> >& cube);
		int closeFile();
	private:
		MATFile *matlabDocument;
		mxArray *pa;

		int dimensions;
		int dimensionLengths[];
};

#endif


