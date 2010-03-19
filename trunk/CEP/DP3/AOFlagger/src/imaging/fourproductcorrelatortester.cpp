#include <AOFlagger/imaging/fourproductcorrelatortester.h>

#include <AOFlagger/imaging/observatorium.h>
#include <AOFlagger/imaging/model.h>
#include <AOFlagger/imaging/uvimager.h>

FourProductCorrelatorTester::FourProductCorrelatorTester(class Model &model, class UVImager &imager, class Observatorium &observatorium)
	: _model(model), _imager(imager), _observatorium(observatorium)
{
}

void FourProductCorrelatorTester::SimulateObservation(num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency)
{
	size_t frequencySteps = 1;

	for(size_t f=0;f<frequencySteps;++f)
	{
		double channelFrequency = frequency + _observatorium.ChannelWidthHz() * f * 256 / frequencySteps;
		for(size_t i=0;i<_observatorium.AntennaCount();++i)
		{
			for(size_t j=0;j<_observatorium.AntennaCount();++j)
			{
				for(size_t k=0;k<_observatorium.AntennaCount();++k)
				{
					for(size_t l=0;l<_observatorium.AntennaCount();++l)
					{
						if(!(i == j && j == k && k == l))
						{
						const AntennaInfo
							&a1 = _observatorium.GetAntenna(i),
							&a2 = _observatorium.GetAntenna(j),
							&a3 = _observatorium.GetAntenna(k),
							&a4 = _observatorium.GetAntenna(l);
		
							SimulateCorrelation(delayDirectionDEC, delayDirectionRA, a1, a2, a3, a4, channelFrequency, 12*60*60, 10.0);
						}
					}
				}
			}
			std::cout << "." << std::flush;
		}
	}
	std::cout << std::endl;
}

void FourProductCorrelatorTester::SimulateCorrelation(num_t delayDirectionDEC, num_t delayDirectionRA, const AntennaInfo &a1, const AntennaInfo &a2, const AntennaInfo &a3, const AntennaInfo &a4, num_t frequency, double totalTime, double integrationTime)
{
	num_t
		x = a1.position.x,
		y = a1.position.y,
		z = a1.position.z,
		dx2 = a2.position.x - x,
		dy2 = a2.position.y - y,
		dz2 = a2.position.z - z,
		dx3 = a3.position.x - x,
		dy3 = a3.position.y - y,
		dz3 = a3.position.z - z,
		dx4 = a4.position.x - x,
		dy4 = a4.position.y - y,
		dz4 = a4.position.z - z,
		combdx = dx3 - dx2 - dx4,
		combdy = dy3 - dy2 - dy4,
		combdz = dz3 - dz2 - dz4;

	num_t wavelength = 1.0L / frequency;
	for(num_t t=0.0;t<totalTime;t+=integrationTime)
	{
		double earthLattitudeApprox = t*(M_PI/12.0/60.0/60.0);
		num_t u, v, rsub, isub, r, i;
		_model.GetUVPosition(u, v, earthLattitudeApprox, delayDirectionDEC, delayDirectionRA, combdx, combdy, combdz, wavelength);

		_model.SimulateAntenna(delayDirectionDEC, delayDirectionRA, 0, 0, 0, frequency, earthLattitudeApprox, r, i);

		_model.SimulateAntenna(delayDirectionDEC, delayDirectionRA, dx2, dy2, dz2, frequency, earthLattitudeApprox, rsub, isub);
		r = r*rsub - (i*-isub);
		i = r*-isub + i*rsub;
		_model.SimulateAntenna(delayDirectionDEC, delayDirectionRA, dx3, dy3, dz3, frequency, earthLattitudeApprox, rsub, isub);
		r = r*rsub - (i*isub);
		i = r*isub + i*rsub;
 		_model.SimulateAntenna(delayDirectionDEC, delayDirectionRA, dx4, dy4, dz4, frequency, earthLattitudeApprox, rsub, isub);
		r = r*rsub - (i*-isub);
		i = r*-isub + i*rsub;
		_imager.SetUVValue(u, v, r, i, 1.0);
		_imager.SetUVValue(-u, -v, r, -i, 1.0);
	}
}
