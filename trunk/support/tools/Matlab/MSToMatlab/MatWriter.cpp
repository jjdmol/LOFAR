#include "MatWriter.h"
#include "mat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace casa;

typedef float dataType;

void plotCube(const Cube<complex<float> >& cube)
{
	uInt nAntenae1 = cube.nrow();
	uInt nAntenae2 = cube.ncolumn();
	uInt nFreq = cube.nplane();

	for(uInt i=0; i<nAntenae1; i++){
		for(uInt j=0; j<nAntenae2; j++)
		{
			if(j >= i)
			{
				complex<float> complexFloat = cube(i,j,1);
				cout << setw(5) << complexFloat << ",";
			}
			else
			{
				cout << setw(6) << " ";
			}
		}
		cout << endl;
	}
}

MatWriter::MatWriter(int dim1, int dim2, int dim3)
{
	dimensions = 3;
	dimensionLengths[0]= dim1;
	dimensionLengths[1]= dim2;
	dimensionLengths[2]= dim3;
	matlabDocument = NULL;

}

MatWriter::~MatWriter()
{
	
}

int MatWriter::openFile(const string& fileName)
{
	if(matlabDocument == NULL)
	{
		cout << "Create new Matlab document..." << fileName << endl << flush;
		matlabDocument = matOpen(fileName.c_str(), "w");
		if (matlabDocument == NULL) {
			cout << "Error creating file " <<  fileName << endl;
			cout << "(Do you have write permission in this directory?)" << endl;
			return(-1);
		}
		cout << "Done." << endl;

		std::cout << "Create " << dimensions << "d array..." << std::flush;
		pa = mxCreateNumericArray(dimensions, dimensionLengths, mxSINGLE_CLASS, mxCOMPLEX);
		if (pa == NULL) {
			printf("%s : Out of memory on line %d\n", __FILE__, __LINE__);
			printf("Unable to create mxArray.\n");
			return(-1);
		}
		std::cout << "Done.\n";
	}
	else
	{
		cout << "Close prevorious document before opening an other!" << endl;
		return -1;
	}
	return 0;
}

int MatWriter::writeCube(const Cube<complex<float> >& cube)
{
	if(matlabDocument == NULL)
	{
		cout << "Open Matlab document before writing to it!" << endl;
		return -1;
	}
// Indexer for 3D matlab array
	int multiDimensionalIndex[3] = {0,0,0};

	// time is 4th dimension
	//multiDimensionalIndex[3] = timeSlot;

	// Amount of antenna1's, antenna2's and frequency bands
	uInt nAntenae1 = cube.nrow();
	uInt nAntenae2 = cube.ncolumn();
	uInt nFreq = cube.nplane();

	cout << "Cube dimensions: " << nAntenae1 << "," << nAntenae2 << "," << nFreq << flush << endl;

	// Indexers for matlab 1D array
	int matlabIndex=0;
	int matlabIndexConjugate=0;

	for(uInt i=0; i<nAntenae1; i++){
		for(uInt j=i; j<nAntenae2; j++){ // Don't miss the j=i assignment, instead of j=0
			for(uInt f=0; f<nFreq; f++)
			{
				multiDimensionalIndex[0] = i;	// Antenne1 is first dimension
				multiDimensionalIndex[1] = j; // Antenne2 is second dimension
				multiDimensionalIndex[2] = f;	// Frequency is third dimension

				// Calculate 3D index in matlab 1D array
				matlabIndex = mxCalcSingleSubscript(pa, dimensions, multiDimensionalIndex);
				
				// Get one value out of 3D cube
				complex<dataType> complexValue = cube(i,j,f);

				// Split in Re and Im values
				dataType Re = real(complexValue);
				dataType Im = imag(complexValue);

				// Copy Re and Im to matlab 1D array
				memcpy((float *)mxGetData(pa)+matlabIndex, (void *)&Re, sizeof(dataType));
				memcpy((float *)mxGetImagData(pa)+matlabIndex, (void *)&Im, sizeof(dataType));

				if(i != j) // Write Complex conjugate when i!=j
				{
					//for the complex conjugate we have to switch antenna 1 and 2
					multiDimensionalIndex[0] = j;
					multiDimensionalIndex[1] = i;

					//Calculate 3D index of complex Conjugate in matlab 1D array
					matlabIndexConjugate = mxCalcSingleSubscript(pa, dimensions, multiDimensionalIndex);

					//create complex conjugate
					dataType ImConjungate = Im * -1;

					//Copy Re and conjugate Im to matlab 1D array
					memcpy((float *)mxGetData(pa)+matlabIndexConjugate, (void *)&Re, sizeof(dataType));
					memcpy((float *)mxGetImagData(pa)+matlabIndexConjugate, (void *)&ImConjungate, sizeof(dataType));
				}
			}
		}
	}
	return 0;
}

int MatWriter::closeFile()
{
	if(matlabDocument == NULL)
	{
		cout << "Cannot close document that is not opened." << endl;
		return -1;
	}
	int status =0;
	std::cout << "Saving..." <<std::flush;
	status = matPutVariableAsGlobal(matlabDocument, "Measurements", pa);
	if (status != 0) {
		printf("Error using matPutVariableAsGlobal\n");
	}

	if (matClose(matlabDocument) != 0) {
		printf("Error closing file.");
	}
	else{
		printf("Done.\n");
	}

	std::cout << "Freeing memory..." << std::flush;
	mxDestroyArray(pa);
	printf("Done.\n");
	matlabDocument = NULL;
	return 0;
}

