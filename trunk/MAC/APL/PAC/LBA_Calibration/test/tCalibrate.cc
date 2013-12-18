#include <APL/LBA_Calibration/lba_calibration.h>
#include <iostream>
#include <fstream>
#include <iterator>


using namespace std;

int driver(int argc, char **argv)
{

    try {
		// get array covariance cube
		ifstream caldata("20080118_083229_acc_512x96x96.dat");
		double *data = new double[2 * 512 * 96 * 96];
		caldata.read(reinterpret_cast<char *>(data), 2 * 512 * 96 * 96 * sizeof(double));
		const mwSize dim[3] = {96, 96, 512};
		mwArray acc(3, dim, mxDOUBLE_CLASS, mxCOMPLEX);
		int dataidx = 0;
		for (int sb = 1; sb <= 512; sb++) {
			for (int idx2 = 1; idx2 <= 96; idx2++) {
				for (int idx1 = 1; idx1 <= 96; idx1++) {
					acc(idx1, idx2, sb).Real() = data[dataidx];
					acc(idx1, idx2, sb).Imag() = data[dataidx + 1];
					dataidx += 2;
				}
			}
		}
		delete[] data;

		// get antenna positions
		ifstream antfile("antpos.dat");
		double *antdata = new double[3 * 48];
		antfile.read(reinterpret_cast<char *>(antdata), 3 * 48 * sizeof(double));
		mwArray antpos(48, 3, mxDOUBLE_CLASS);
		antpos.SetData(antdata, 3 * 48);
		delete[] antdata;

		// get source positions for test data
		ifstream srcfile("srcpos.dat");
		double *srcposdata = new double[3 * 3];
		srcfile.read(reinterpret_cast<char *>(srcposdata), 3 * 3 * sizeof(double));
		mwArray srcpos(3, 3, mxDOUBLE_CLASS);
		srcpos.SetData(srcposdata, 3 * 3);
		delete[] srcposdata;

		// get center frequencies of subband
		ifstream freqfile("frequencies.dat");
		double *freqdata = new double[512];
		freqfile.read(reinterpret_cast<char *>(freqdata), 512 * sizeof(double));
		mwArray freq(512, 1, mxDOUBLE_CLASS);
		freq.SetData(freqdata, 512);
		delete[] freqdata;

		// up to this point, the code has been verified

		// call calibrate-function
		mwArray calresult;
		mwArray mflags;
		lba_calibration(2, calresult, mflags, acc, antpos, srcpos, freq);

		// retrieve data
		double *cal_real = new double[512 * 96];
		double *cal_imag = new double[512 * 96];
		double *flags = new double[512 * 96];
		calresult.Real().GetData(cal_real, 512 * 96);
		calresult.Imag().GetData(cal_imag, 512 * 96);
		mflags.GetData(flags, 512 * 96);
		ofstream calfile("calresult.dat");
		calfile.write(reinterpret_cast<char *>(cal_real), 512 * 96 * sizeof(double));
		calfile.write(reinterpret_cast<char *>(cal_imag), 512 * 96 * sizeof(double));
		ofstream flagfile("flags.dat");
		flagfile.write(reinterpret_cast<char *>(flags), 512 * 96 * sizeof(double));
		delete[] flags;
		delete[] cal_imag;
		delete[] cal_real;
    }
    catch(const mwException& e) {
		cerr << e.what() << endl;
		return -3;
    }
    catch(...) {
		cerr << "Unexpected error" << endl;
		return -4;
    }

    return 0;
}

int main()
{
    mclmcrInitialize();

    if (!mclInitializeApplication(NULL, 0)) {
		cerr << "Failed to initialize the MCR" << endl;
		return -1;
    }
    if (!liblba_calibrationInitialize()) {
		cerr << "Failed to initialize liblba_calibration.so" << endl;
		return -2;
    }
    cout << "MCR and liblba_calibration.so successfully initialized" << endl;

	cout << "@@@@@ RUN 1 @@@@@" << endl;
    mclRunMain((mclMainFcnType)driver, 0, NULL);
	cout << "@@@@@ RUN 2 @@@@@" << endl;
    mclRunMain((mclMainFcnType)driver, 0, NULL);

    liblba_calibrationTerminate();
    mclTerminateApplication();

    cout << "All garbage has been successfully collected" << endl;

	return (0);
}
