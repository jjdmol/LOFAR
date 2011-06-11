
#include <cmath>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/util/aologger.h>

#include "eigenvalue.h"

/**
 * This class performs a horizontal Eigenvalue Decomposition. Horizon
 */
class VertEVD
{
	public:
		static void Perform(TimeFrequencyData &data)
		{
			if(data.PolarisationCount() != 1)
				throw std::runtime_error("Can not decompose multipolarization data");
			if(data.PhaseRepresentation() != TimeFrequencyData::ComplexRepresentation)
				throw std::runtime_error("Can only decompose complex data");
			
			Image2DCPtr
				real = data.GetRealPart(),
				imaginary = data.GetImaginaryPart();
			Image2DPtr
				outReal = Image2D::CreateZeroImagePtr(real->Width(), real->Height()),
				outImaginary = Image2D::CreateZeroImagePtr(real->Width(), real->Height());
				
			// Since the number of vertical elements e = n(n-1)/2 , solving for e
			// results in:
			// e = n(n-1)/2
			// e = 0.5n^2 - 0.5n
			// 0 = 0.5n^2 - 0.5n - e
			// n = -(-0.5) +- sqrt(0.25 - 4 x 0.5 x -e)
			// n = 0.5 + sqrt(0.25 + 2e)
			const size_t n = floor(0.5 + sqrt(0.25 + 2 * real->Height()));
			const size_t skipped = real->Height()-((n * (n-1)) / 2);
			if(n > 0)
			{
				AOLogger::Warn << "In vertical eigenvalue decomposition: height did not correspond with an exact triangle:\n"
				<< skipped << " values of height " << (real->Height()) << " were skipped to create matrix size " << n << "\n.";
			}
			Image2DPtr
				realMatrix = Image2D::CreateEmptyImagePtr(n, n),
				imaginaryMatrix = Image2D::CreateEmptyImagePtr(n, n);
			
			for(unsigned t=0;t<real->Width();++t)
			{
				double diagonal[n];
				for(unsigned i=0;i<n;++i)
					diagonal[i] = 0.0;
				
				for(unsigned diagIteration=0;diagIteration<5;++diagIteration)
				{
					unsigned index = 0;
					for(unsigned y=0;y<n;++y)
					{
						realMatrix->SetValue(y, y, diagonal[y]);
						imaginaryMatrix->SetValue(y, y, diagonal[y]);
						
						for(unsigned x=y+1;x<n;++x)
						{
							realMatrix->SetValue(x, y, real->Value(t, index));
							imaginaryMatrix->SetValue(x, y, imaginary->Value(t, index));

							realMatrix->SetValue(y, x, real->Value(t, index));
							imaginaryMatrix->SetValue(y, x, -imaginary->Value(t, index));


							++index;
						}
					}
					
					if(t == 262)
					{
						AOLogger::Debug << "Input to EVD:\n";
						for(unsigned y=0;y<realMatrix->Height();++y) {
							for(unsigned x=0;x<realMatrix->Width();++x) {
								AOLogger::Debug << realMatrix->Value(x, y) << ' ';
							}
							AOLogger::Debug << '\n';
						}
					}
					
					Eigenvalue::Remove(realMatrix, imaginaryMatrix, t==262);
					
					if(t == 262)
					{
						AOLogger::Debug << "Output to EVD:\n";
						for(unsigned y=0;y<realMatrix->Height();++y) {
							for(unsigned x=0;x<realMatrix->Width();++x) {
								AOLogger::Debug << realMatrix->Value(x, y) << ' ';
							}
							AOLogger::Debug << '\n';
						}
					}
					
					for(unsigned i=0;i<n;++i)
						diagonal[i] = realMatrix->Value(i, i);
				}

				unsigned index = 0;
				for(unsigned y=0;y<n;++y)
				{
					for(unsigned x=y+1;x<n;++x)
					{
						outReal->SetValue(t, index, realMatrix->Value(x, y));
						outImaginary->SetValue(t, index, imaginaryMatrix->Value(x, y));
						++index;
					}
				}
				
				while(index < real->Height())
				{
					outReal->SetValue(t, index, 0.0);
					outImaginary->SetValue(t, index, 0.0);
					++index;
				}
			}
			
			data.SetImage(0, outReal);
			data.SetImage(1, outImaginary);
		}
	private:
		VertEVD() { }
};
