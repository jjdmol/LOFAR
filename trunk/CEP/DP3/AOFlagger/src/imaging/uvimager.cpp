/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/util/integerdomain.h>
#include <AOFlagger/util/stopwatch.h>
#include <AOFlagger/util/ffttools.h>

UVImager::UVImager(unsigned long xRes, unsigned long yRes, ImageKind imageKind) : _xRes(xRes), _yRes(yRes), _xResFT(xRes), _yResFT(yRes), _uvReal(0), _uvImaginary(0), _uvWeights(0), _uvFTReal(0), _uvFTImaginary(0), _antennas(0), _fields(0), _imageKind(imageKind), _invertFlagging(false), _directFT(false), _ignoreBoundWarnings(false)
{
	_uvScaling = 0.00025; // testing
	Empty();
}


UVImager::~UVImager()
{
	Clear();
}

void UVImager::Clear()
{
	if(_uvReal != 0) {
		delete _uvReal;
		delete _uvImaginary;
		delete _uvWeights;
		_uvReal = 0;
		_uvImaginary = 0;
		_uvWeights = 0;
	}
	if(_antennas != 0) {
		delete[] _antennas;
		_antennas = 0;
	}
	if(_fields != 0) {
		delete [] _fields;
		_fields = 0;
	}
	if(_uvFTReal != 0) {
		delete _uvFTReal;
		delete _uvFTImaginary;
		_uvFTReal = 0;
		_uvFTImaginary = 0;
	}
}

void UVImager::Empty()
{
	Clear();
	_uvReal = Image2D::CreateZeroImage(_xRes, _yRes);
	_uvImaginary = Image2D::CreateZeroImage(_xRes, _yRes);
	_uvWeights = Image2D::CreateZeroImage(_xRes, _yRes);
}

void UVImager::Image(class MeasurementSet &measurementSet, unsigned band)
{
	unsigned frequencyCount = measurementSet.FrequencyCount();
	IntegerDomain frequencies(0, frequencyCount);
	_measurementSet = &measurementSet;
	_band = _measurementSet->GetBandInfo(band);

	Image(frequencies);
}

void UVImager::Image(class MeasurementSet &measurementSet, unsigned band, const IntegerDomain &frequencies)
{
	_measurementSet = &measurementSet;
	_band = _measurementSet->GetBandInfo(band);

	Image(frequencies);
}

void UVImager::Image(const IntegerDomain &frequencies)
{
	Empty();

	_antennaCount = _measurementSet->AntennaCount();
	_antennas = new AntennaInfo[_antennaCount];
	for(unsigned i=0;i<_antennaCount;++i)
		_antennas[i] = _measurementSet->GetAntennaInfo(i);
	
	_fieldCount = _measurementSet->FieldCount();
	_fields = new FieldInfo[_fieldCount];
	for(unsigned i=0;i<_fieldCount;++i)
		_fields[i] = _measurementSet->GetFieldInfo(i);

	unsigned parts = (frequencies.ValueCount()-1)/48 + 1;
	for(unsigned i=0;i<parts;++i) {
		std::cout << "Imaging " << i << "/" << parts << ":" << frequencies.Split(parts, i).ValueCount() << " frequencies..." << std::endl;
		Image(frequencies.Split(parts, i), IntegerDomain(0, _antennaCount), IntegerDomain(0, _antennaCount));
	}
}

/**
 * Add several frequency channels to the uv plane for several combinations
 * of antenna pairs.
 */
void UVImager::Image(const IntegerDomain &frequencies, const IntegerDomain &antenna1Domain, const IntegerDomain &antenna2Domain)
{
	_scanCount = _measurementSet->MaxScanIndex()+1;
	std::cout << "Requesting " << frequencies.ValueCount() << " x " << antenna1Domain.ValueCount() << " x " << antenna2Domain.ValueCount() << " x "
		<< _scanCount << " x " << sizeof(SingleFrequencySingleBaselineData) << " = "
		<< (frequencies.ValueCount() * antenna1Domain.ValueCount() * antenna2Domain.ValueCount() * _scanCount * sizeof(SingleFrequencySingleBaselineData) / (1024*1024))
		<< "MB of memory..." << std::endl;
	SingleFrequencySingleBaselineData ****data =
		new SingleFrequencySingleBaselineData***[frequencies.ValueCount()];
	for(unsigned f = 0;f<frequencies.ValueCount();++f) {
		data[f] =	new SingleFrequencySingleBaselineData**[antenna1Domain.ValueCount()];
		for(unsigned a1 = 0;a1<antenna1Domain.ValueCount();++a1) {
			data[f][a1] = new SingleFrequencySingleBaselineData*[antenna2Domain.ValueCount()];
			for(unsigned a2 = 0;a2<antenna2Domain.ValueCount();++a2) {
				data[f][a1][a2] = new SingleFrequencySingleBaselineData[_scanCount];
				for(unsigned scan = 0; scan<_scanCount;++scan) {
					data[f][a1][a2][scan].flag = true;
					data[f][a1][a2][scan].available = false;
				}
			}	
		}
	}

	std::cout << "Reading all data for " << frequencies.ValueCount() << " frequencies..." << std::flush;
	Stopwatch stopwatch(true);
	MSIterator iterator(*_measurementSet);
	size_t rows = iterator.TotalRows();
	for(unsigned i=0;i<rows;++i,++iterator) {
		unsigned a1 = iterator.Antenna1();
		unsigned a2 = iterator.Antenna2();
		if(antenna1Domain.IsIn(a1) && antenna2Domain.IsIn(a2)) {
			unsigned scan = iterator.ScanNumber();
			unsigned index1 = antenna1Domain.Index(a1);
			unsigned index2 = antenna1Domain.Index(a2);
			int field = iterator.Field();
			double time = iterator.Time();
			casa::Array<casa::Complex>::const_iterator cdI = iterator.CorrectedDataIterator();
			casa::Array<bool>::const_iterator fI = iterator.FlagIterator();
			for(int f=0;f<frequencies.GetValue(0);++f) { ++fI; ++fI; ++fI; ++fI; ++cdI; ++cdI; ++cdI; ++cdI; }
			for(unsigned f=0;f<frequencies.ValueCount();++f) {
				SingleFrequencySingleBaselineData &curData = data[f][index1][index2][scan];
				casa::Complex xxData = *cdI;
				++cdI; ++cdI; ++cdI;
				casa::Complex yyData = *cdI;
				++cdI;
				curData.data = xxData + yyData;
				bool flagging = *fI;
				++fI; ++fI; ++fI;
				flagging = flagging || *fI;
				++fI;
				curData.flag = flagging;
				curData.field = field;
				curData.time = time;
				curData.available = true;
			}
		}
	}
	stopwatch.Pause();
	std::cout << "DONE in " << stopwatch.ToString() << " (" << (stopwatch.Seconds() / (antenna1Domain.ValueCount() * antenna1Domain.ValueCount())) << "s/antenna)" << std::endl;

	std::cout << "Imaging..." << std::flush;
	stopwatch.Reset();
	stopwatch.Start();
	for(unsigned f = 0;f<frequencies.ValueCount();++f) {
		for(unsigned a1 = 0;a1<antenna1Domain.ValueCount();++a1) {
			for(unsigned a2 = 0;a2<antenna2Domain.ValueCount();++a2) {
				Image(frequencies.GetValue(f), _antennas[antenna1Domain.GetValue(a1)], _antennas[antenna2Domain.GetValue(a2)], data[f][a1][a2]);
			}
		}
	}
	stopwatch.Pause();
	std::cout << "DONE in " << stopwatch.ToString() << " (" << (stopwatch.Seconds() / (antenna1Domain.ValueCount() * antenna1Domain.ValueCount())) << "s/antenna)" << std::endl;

	// free data
	for(unsigned f = 0;f<frequencies.ValueCount();++f) {
		for(unsigned a1 = 0;a1<antenna1Domain.ValueCount();++a1) {
			for(unsigned a2 = 0;a2<antenna2Domain.ValueCount();++a2) {
				delete[] data[f][a1][a2];
			}
			delete[] data[f][a1];
		}
		delete[] data[f];
	}
	delete[] data;
}

void UVImager::Image(unsigned frequencyIndex, AntennaInfo &antenna1, AntennaInfo &antenna2, SingleFrequencySingleBaselineData *data)
{
	long double frequency = _band.channels[frequencyIndex].frequencyHz;
	long double speedOfLight = 299792458.0L;
	AntennaCache cache;
	cache.wavelength = speedOfLight / frequency;

	// dx, dy, dz is the baseline
	cache.dx = antenna1.position.x - antenna2.position.x;
	cache.dy = antenna1.position.y - antenna2.position.y;
	cache.dz = antenna1.position.z - antenna2.position.z;

	//MSIterator iterator(*_measurementSet);
	//bool baselineProvided = false;
	//Stopwatch stopwatch(false);
	//Stopwatch calcTimer(false);
	for(unsigned i=0;i<_scanCount;++i) {
			if(data[i].available) {
				//if(!baselineProvided) {
					//std::cout << "Now processing frequency " << frequency << " Hz (index " << frequencyIndex << "), correlation between " << antenna1.station << " " << antenna1.name << " and " << antenna2.station << " " << antenna2.name << "..." << std::flush;
					//baselineProvided = true;
					//stopwatch.Start();
				//}
				switch(_imageKind) {
				case Homogeneous:
				if(!data[i].flag) {
					long double u,v;
					GetUVPosition(u, v, data[i], cache);
					SetUVValue(u, v, data[i].data.real(), data[i].data.imag(), 1.0);
					SetUVValue(-u, -v, data[i].data.real(), -data[i].data.imag(), 1.0);
					//calcTimer.Pause();
				} 
				break;
				case Flagging:
				if((data[i].flag && !_invertFlagging) ||
						(!data[i].flag && _invertFlagging)) {
					long double u,v;
					GetUVPosition(u, v, data[i], cache);
					SetUVValue(u, v, 1, 0, 1.0);
					SetUVValue(-u, -v, 1, 0, 1.0);
				}
				break;
			}
		}
	}
}

void UVImager::Image(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData, unsigned frequencyIndex)
{
	if(_uvReal == 0)
	{
		Empty();
	}

	//long double frequency = metaData->Band().channels[frequencyIndex].frequencyHz;
	//long double speedOfLight = 299792458.0L;
	//AntennaCache cache;
	//cache.wavelength = speedOfLight / frequency;

	// dx, dy, dz is the baseline
	//cache.dx = metaData->Antenna1().position.x - metaData->Antenna2().position.x;
	//cache.dy = metaData->Antenna1().position.y - metaData->Antenna2().position.y;
	//cache.dz = metaData->Antenna1().position.z - metaData->Antenna2().position.z;

	Image2DCPtr
		real = data.GetRealPart(),
		imaginary = data.GetImaginaryPart();
	Mask2DCPtr
		flags = data.GetSingleMask();

	for(unsigned i=0;i<data.ImageWidth();++i) {
		switch(_imageKind) {
			case Homogeneous:
			if(flags->Value(i, frequencyIndex)==0.0L) {
				long double
					vr = real->Value(i, frequencyIndex),
					vi = imaginary->Value(i, frequencyIndex);
				if(std::isfinite(vr) && std::isfinite(vi))
				{
					long double u,v;
					GetUVPosition(u, v, i, frequencyIndex, metaData);
					SetUVValue(u, v, vr, vi, 1.0);
					SetUVValue(-u, -v, vr, -vi, 1.0);
				}
				//calcTimer.Pause();
			} 
			break;
			case Flagging:
			if((flags->Value(i, frequencyIndex)!=0.0L && !_invertFlagging) ||
					(flags->Value(i, frequencyIndex)==0.0L && _invertFlagging)) {
				long double u,v;
				GetUVPosition(u, v, i, frequencyIndex, metaData);
				SetUVValue(u, v, 1, 0, 1.0);
				SetUVValue(-u, -v, 1, 0, 1.0);
			}
			break;
		}
	}
}

void UVImager::ApplyWeightsToUV()
{
	for(size_t y=0;y<_uvReal->Height();++y) {
		for(size_t x=0;x<_uvReal->Width();++x) {
			num_t weight = _uvWeights->Value(x, y);
			if(weight != 0.0)
			{
				_uvReal->SetValue(x, y, _uvReal->Value(x, y) / weight);
				_uvImaginary->SetValue(x, y, _uvImaginary->Value(x, y) / weight);
				_uvWeights->SetValue(x, y, 1.0);
			} 
		}
	}
	if(_uvFTReal != 0) {
		delete _uvFTReal;
		delete _uvFTImaginary;
		_uvFTReal = 0;
		_uvFTImaginary = 0;
	}
}

void UVImager::SetUVValue(long double u, long double v, long double r, long double i, long double weight)
{
	 // Nearest neighbour interpolation
	long uPos = (long) floorl(u*_uvScaling*_xRes+0.5L) + (_xRes/2);
	long vPos = (long) floorl(v*_uvScaling*_yRes+0.5L) + (_yRes/2);
	if(uPos>=0 && uPos<(long) _xRes && vPos>=0 && vPos<(long) _yRes) {
		_uvReal->AddValue(uPos, vPos, r);
		_uvImaginary->AddValue(uPos, vPos, i);
		_uvWeights->AddValue(uPos, vPos, weight);
	} else {
		if(!_ignoreBoundWarnings)
		{
			std::cout << "Warning! Baseline outside uv window (" << uPos << "," << vPos << ")." << 
			"(subsequent out of bounds warnings will not be noted)" << std::endl;
			_ignoreBoundWarnings = true;
		}
	}
	// Linear interpolation
	/*long uPos = (long) floor(u*_uvScaling*_xRes+0.5L) + _xRes/2;
	long vPos = (long) floor(v*_uvScaling*_yRes+0.5L) + _yRes/2;
	if(uPos>=0 && uPos<(long) _xRes && vPos>=0 && vPos<(long) _yRes) {
		long double dx = (long double) uPos - (_xRes/2) - (u*_uvScaling*_xRes+0.5L);
		long double dy = (long double) vPos - (_yRes/2) - (v*_uvScaling*_yRes+0.5L);
		long double distance = sqrtl(dx*dx + dy*dy);
		if(distance > 1.0) distance = 1.0;
		weight *= distance;
		_uvReal->AddValue(uPos, vPos, r*weight);
		_uvImaginary->AddValue(uPos, vPos, i*weight);
		_uvWeights->AddValue(uPos, vPos, weight);
	} else {
		std::cout << "Warning! Baseline outside uv window (" << uPos << "," << vPos << ")." << std::endl;
	}*/
}

void UVImager::SetUVFTValue(long double u, long double v, long double r, long double i, long double weight)
{
	for(size_t iy=0;iy<_yResFT;++iy)
	{
		for(size_t ix=0;ix<_xResFT;++ix)
		{
			long double x = ((long double) ix - (_xResFT/2)) / _uvScaling * _uvFTReal->Width();
			long double y = ((long double) iy - (_yResFT/2)) / _uvScaling * _uvFTReal->Height();
			// Calculate F(x,y) += f(u, v) e ^ {i 2 pi (x u + y v) } 
			long double fftRotation = (u * x + v * y) * -2.0L * M_PIl;
			long double fftCos = cosl(fftRotation), fftSin = sinl(fftRotation);
			_uvFTReal->AddValue(ix, iy, (fftCos * r - fftSin * i) * weight);
			_uvFTImaginary->AddValue(ix, iy, (fftSin * r + fftCos * i) * weight);
		}
	}

}

void UVImager::PerformFFT()
{
	if(_uvFTReal == 0)
	{
		_uvFTReal = Image2D::CreateZeroImage(_xRes, _yRes);
		_uvFTImaginary = Image2D::CreateZeroImage(_xRes, _yRes);
	}
	FFTTools::CreateFFTImage(*_uvReal, *_uvImaginary, *_uvFTReal, *_uvFTImaginary);
}

void UVImager::GetUVPosition(long double &u, long double &v, size_t timeIndex, size_t frequencyIndex, TimeFrequencyMetaDataCPtr metaData)
{
	long double frequency = metaData->Band().channels[frequencyIndex].frequencyHz;
	u = metaData->UVW()[timeIndex].u * frequency / 299792458.0L;
	v = metaData->UVW()[timeIndex].v * frequency / 299792458.0L;
	return;
	const Baseline &baseline = metaData->Baseline();
	long double delayDirectionRA = metaData->Field().delayDirectionRA;
	long double delayDirectionDec = metaData->Field().delayDirectionDec;
	double time = metaData->ObservationTimes()[timeIndex];

	long double pointingLattitude = delayDirectionRA;
	long double earthLattitudeAngle = Date::JDToHourOfDay(Date::AipsMJDToJD(time))*M_PIl/12.0L;

	// Rotate baseline plane towards source, first rotate around x axis, then around z axis
	long double raRotation = earthLattitudeAngle - pointingLattitude + M_PIl*0.5L;
	long double raCos = cosl(-raRotation);
	long double raSin = sinl(-raRotation);

	long double dx = baseline.antenna1.x - baseline.antenna2.x;
	long double dy = baseline.antenna1.y - baseline.antenna2.y;
	long double dz = baseline.antenna1.z - baseline.antenna2.z;

	long double decCos = cosl(delayDirectionDec);
	long double decSin = sinl(delayDirectionDec);

	long double
		du = -dx * raCos * decSin - dy * raSin - dz * raCos * decCos,
		dv = -dx * raSin * decSin + dy * raCos - dz * raSin * decCos;

  /*
	long double dxProjected = tmpCos*dx - tmpSin*dy;
	long double tmpdy = tmpSin*dx + tmpCos*dy;

	long double dyProjected = tmpCos*tmpdy - tmpSin*dz;*/

	// du = dx*cos(ra) - dy*sin(ra)
	// dv = ( dx*sin(ra) + dy*cos(ra) ) * cos(-dec) - dz * sin(-dec)
	// Now, the newly projected positive z axis of the baseline points to the field
	long double baselineLength = sqrtl(du*du + dv*dv);

	long double baselineAngle;
	if(baselineLength == 0.0)
		baselineAngle = 0.0;
	else {
		baselineLength *= frequency / 299792458.0L;
		if(du > 0.0L)
			baselineAngle = atanl(du/dv);
		else
			baselineAngle = M_PIl - atanl(du/-dv);
	}
	u = cosl(baselineAngle)*baselineLength;
	v = -sinl(baselineAngle)*baselineLength;

	std::cout << "Calced: " << u << "," << v
		<< ", ori: " << metaData->UVW()[timeIndex].u << "," << metaData->UVW()[timeIndex].v << "(," << metaData->UVW()[timeIndex].w << ")\n";
}

void UVImager::GetUVPosition(long double &u, long double &v, const SingleFrequencySingleBaselineData &data, const AntennaCache &cache)
{
	unsigned field = data.field;
	long double pointingLattitude = _fields[field].delayDirectionRA;

	//calcTimer.Start();
	long double earthLattitudeAngle = Date::JDToHourOfDay(Date::AipsMJDToJD(data.time))*M_PIl/12.0L;

	//long double pointingLongitude = _fields[field].delayDirectionDec; //not used

	// Rotate baseline plane towards source, first rotate around z axis, then around x axis
	long double raRotation = earthLattitudeAngle - pointingLattitude + M_PIl*0.5L;
	long double tmpCos = cosl(raRotation);
	long double tmpSin = sinl(raRotation);

	long double dxProjected = tmpCos*cache.dx - tmpSin*cache.dy;
	long double tmpdy = tmpSin*cache.dx + tmpCos*cache.dy;

	tmpCos = _fields[field].delayDirectionDecNegCos; // cosl(-pointingLongitude);
	tmpSin = _fields[field].delayDirectionDecNegSin; //sinl(-pointingLongitude);
	long double dyProjected = tmpCos*tmpdy - tmpSin*cache.dz;
	// long double dzProjected = tmpSin*tmpdy + tmpCos*dzAnt; // we don't need it

	// Now, the newly projected positive z axis of the baseline points to the field

	long double baselineLength = sqrtl(dxProjected*dxProjected + dyProjected*dyProjected);
	
	long double baselineAngle;
	if(baselineLength == 0.0L)
		baselineAngle = 0.0L;
	else {
		baselineLength /= cache.wavelength;
		if(dxProjected > 0.0L)
			baselineAngle = atanl(dyProjected/dxProjected);
		else
			baselineAngle = M_PI - atanl(dyProjected/-dxProjected);
	}
		

	u = cosl(baselineAngle)*baselineLength;
	v = -sinl(baselineAngle)*baselineLength;
}

double UVImager::GetFringeStopFrequency(long double time, const Baseline &baseline, long double delayDirectionRA, long double delayDirectionDec, long double frequency)
{
	// earthspeed = rad / sec
	const long double earthSpeed = 2.0L * M_PI / (24.0L * 60.0L * 60.0L);
	long double earthLattitudeAngle =
		Date::JDToHourOfDay(Date::AipsMJDToJD(time))*M_PI/12.0L;
	//long double u, v;
	//GetUVPosition(u, v, baseline, time, delayDirectionRA, delayDirectionDec, frequency);
	//return
	//	earthSpeed * (u * sinl(delayDirectionRA-earthLattitudeAngle) + v * cosl(delayDirectionRA-earthLattitudeAngle)) * cosl(delayDirectionDec);
	long double raSin = sinl(-delayDirectionRA - earthLattitudeAngle);
	long double raCos = cosl(-delayDirectionRA - earthLattitudeAngle);
	long double dx = baseline.antenna2.x - baseline.antenna1.x;
	long double dy = baseline.antenna2.y - baseline.antenna1.y;
	//long double dz = baseline.antenna2.z - baseline.antenna1.z;
	long double wavelength = 299792458.0L / frequency;
	/*std::cout << "Angle=" <<
		180.0L / M_PI * acosl(((dx * raCos - dy * raSin) * cosl(delayDirectionDec) + dz*sinl(delayDirectionDec)) / sqrtl(dx*dx + dy*dy + dz*dz))
		<< std::endl; 
	std::cout << "delay=" <<
		((dx * raCos - dy * raSin) * cosl(delayDirectionDec) + dz*sinl(delayDirectionDec)) << "m"
		<< std::endl;
	std::cout << "ddelay/dt=" <<
		(earthSpeed * (dx*raSin + dy*raCos) * cosl(delayDirectionDec))
		<< "m/s" << std::endl;*/
	return (earthSpeed * (dx*raSin + dy*raCos) * cosl(delayDirectionDec)) / wavelength; 
}

double UVImager::GetFringeCount(size_t timeIndexStart, size_t timeIndexEnd, unsigned channelIndex, const TimeFrequencyMetaDataCPtr metaData)
{
	return metaData->UVW()[timeIndexEnd].w - metaData->UVW()[timeIndexStart].w * metaData->Band().channels[channelIndex].frequencyHz / 299792458.0L;
	double
		timeStart = metaData->ObservationTimes()[timeIndexStart],
		timeEnd = metaData->ObservationTimes()[timeIndexEnd];
	long double earthLattitudeAngleStart =
		Date::JDToHourOfDay(Date::AipsMJDToJD(timeStart))*M_PI/12.0L;
	long double earthLattitudeAngleEnd =
		Date::JDToHourOfDay(Date::AipsMJDToJD(timeEnd))*M_PI/12.0L;
	long double wavelength = 299792458.0L / metaData->Band().channels[channelIndex].frequencyHz;
	long double dx = metaData->Antenna2().position.x - metaData->Antenna1().position.x;
	long double dy = metaData->Antenna2().position.y - metaData->Antenna1().position.y;
	long double delayDirectionRA = metaData->Field().delayDirectionRA;
	long double raSinStart = sinl(-delayDirectionRA - earthLattitudeAngleStart);
	long double raCosStart = cosl(-delayDirectionRA - earthLattitudeAngleStart);
	long double raSinEnd = sinl(-delayDirectionRA - earthLattitudeAngleEnd);
	long double raCosEnd = cosl(-delayDirectionRA - earthLattitudeAngleEnd);
	long double decCos = cosl(metaData->Field().delayDirectionDec);
	// term "+ dz * decCos" is eliminated because of subtraction
	long double fringeCount =
		( (dx*raCosStart - dy*raSinStart)
		-
		(dx*raCosEnd - dy*raSinEnd) ) * (-decCos) / wavelength;
	//std::cout << "Fringecount = " << fringeCount << " within " <<timeStart << "-" << timeEnd << "=" << (timeEnd-timeStart) << std::endl; 
	return fringeCount;
}

double UVImager::GetIntegratedFringeStopFrequency(long double timeStart, long double timeEnd, const Baseline &baseline, long double delayDirectionRA, long double delayDirectionDec, long double frequency, size_t steps)
{
	long double fringeCount = 0.0L;
	long double step = (timeEnd-timeStart)/(long double) steps;
	timeEnd += step*0.5;
	for(long double time=timeStart;time<timeEnd;time += step)
	{
		long double fringeFrequency = GetFringeStopFrequency(time, baseline, delayDirectionRA, delayDirectionDec, frequency);
		fringeCount += step*fabsl(fringeFrequency);
	}

	return fringeCount / (timeEnd-timeStart);
}

void UVImager::InverseImage(class MeasurementSet &prototype, unsigned /*band*/, const Image2D &/*uvReal*/, const Image2D &/*uvImaginary*/, unsigned antenna1Index, unsigned antenna2Index)
{
	_timeFreq = Image2D::CreateEmptyImage(prototype.MaxScanIndex()+1, prototype.FrequencyCount());
	AntennaInfo antenna1, antenna2;
	antenna1 = prototype.GetAntennaInfo(antenna1Index);
	antenna2 = prototype.GetAntennaInfo(antenna2Index);
	//Image2D *temp = Image2D::CreateEmptyImage(uvReal.Width(), uvReal.Height());
}
