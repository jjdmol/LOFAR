#ifndef SPATIALMATRIXMETADATA_H
#define SPATIALMATRIXMETADATA_H

#include "antennainfo.h"

class SpatialMatrixMetaData
{
	public:
		SpatialMatrixMetaData(size_t antennaCount)
			: _antennaCount(antennaCount), _antennae(new AntennaInfo[antennaCount]),
				_uvw(new struct UVW*[antennaCount]), _frequency(0.0)
		{
			for(size_t i=0;i<_antennaCount;++i)
				_uvw[i] = new struct UVW[_antennaCount];
		}
		SpatialMatrixMetaData(const SpatialMatrixMetaData &source)
			: _antennaCount(source._antennaCount),
			_antennae(new AntennaInfo[_antennaCount]),
			_uvw(new struct UVW*[_antennaCount]),
			_frequency(source._frequency)
		{
			for(size_t i=0;i<_antennaCount;++i)
			{
				_uvw[i] = new struct UVW[_antennaCount];
				_antennae[i] = source._antennae[i];
				for(size_t j=0;j<_antennaCount;++j)
					_uvw[i][j] = source._uvw[i][j];
			}
		}
		~SpatialMatrixMetaData()
		{
			for(size_t i=0;i<_antennaCount;++i)
				delete[] _uvw[i];
			delete[] _uvw;
			delete[] _antennae;
		}
		void SetAntenna(unsigned index, const AntennaInfo &antenna)
		{
			_antennae[index] = antenna;
		}
		const AntennaInfo &Antenna(unsigned index) const
		{
			return _antennae[index];
		}
		void SetUVW(unsigned a1, unsigned a2, const struct UVW &uvw)
		{
			_uvw[a2][a1] = uvw;
		}
		const struct UVW &UVW(unsigned a1, unsigned a2) const
		{
			return _uvw[a2][a1];
		}
		size_t AntennaCount() const
		{
			return _antennaCount;
		}
		void SetFrequency(num_t frequency)
		{
			_frequency = frequency;
		}
		num_t Frequency() const
		{
			return _frequency;
		}
	private:
		size_t _antennaCount;
		AntennaInfo *_antennae;
		struct UVW **_uvw;
		num_t _frequency;
};

#endif // SPATIALMATRIXMETADATA_H
