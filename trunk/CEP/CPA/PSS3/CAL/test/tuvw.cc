#include <Common/lofar_complex.h>
#include <Common/lofar_iostream.h>
#include <casa/BasicSL/Constants.h>

int main()
{
    double pi = C::pi;
    double u1 = 340567.89;
    double u2 = 340501.73;
    double v1 = 378235.1;
    double v2 = 377352.821;
    double w1 = 342345.2;
    double w2 = 342431.75;
    double l = 0.238;
    double m = 0.531;
    double n = sqrt(1-l*l-m*m);
    double f0 = 234500000.;
    double df = 1000000.;
    double nchan = 1024;
    double c = 300000000.;   //lightspeed

    double s1 = ((u1*l + v1*m + w1*n) * 2 * pi / (c / 1000000));
    double s2 = ((u2*l + v2*m + w2*n) * 2 * pi / (c / 1000000));
    complex<double> d01 (cos(s1*(f0/1000000)), sin(s1*(f0/1000000)));
    complex<double> d02 (cos(s2*(f0/1000000)), sin(s2*(f0/1000000)));
    complex<double> dd1 (cos(s1*(df/1000000)), sin(s1*(df/1000000)));
    complex<double> dd2 (cos(s2*(df/1000000)), sin(s2*(df/1000000)));
    complex<double> dd = dd1 / dd2;
    complex<double> dftb = d01 / (d02 * n);
    double f = f0;
    double s = (u1-u2)*l + (v1-v2)*m + (w1-w2)*n;
    double sc = s1 - s2;
    for (int ch=0; ch<nchan; ch++) {
	complex<double> dfta = exp(complex<double>(0, 2 * pi * s / (c/f))) / n;
	complex<double> dftd = exp(complex<double>(0, 2 * pi * s / (c/1000000) * (f/1000000))) / n;
	complex<double> dftc = exp(complex<double>(0, sc * (f/1000000))) / n;
	cout << dfta << ' ' << dftb << ' ' << dftc << ' ' << dftd << endl;
	f = f + df;
	dftb = dftb * dd;
    }
}
