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
#include <AOFlagger/rfi/rfiplots.h>

#include <cmath>
#include <iostream>

#include <AOFlagger/util/plot.h>
#include <AOFlagger/util/multiplot.h>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/rfi/sinusfitter.h>
#include <AOFlagger/rfi/thresholdtools.h>
#include <AOFlagger/rfi/rfistatistics.h>

void RFIPlots::Bin(Image2DCPtr image, Mask2DCPtr mask, std::vector<size_t> &valuesOutput, std::vector<long double> &binsOutput, size_t binCount, long double start, long double end, long double factor, long double stretch) throw()
{
	const long double min = start==end ? ThresholdTools::MinValue(image, mask) : start;
	const long double max = start==end ? ThresholdTools::MaxValue(image, mask) : end;
	const long double binsize = (max-min) / binCount;
	valuesOutput.resize(binCount);
	binsOutput.resize(binCount);
	for(size_t i=0;i<binCount;++i) {
		valuesOutput[i] = 0;
		binsOutput[i] = (binsize * ((long double) i + 0.5)) + min;
	}
	for(size_t y=0;y<image->Height();++y) {
		for(size_t x=0;x<image->Width();++x) {
			if(!mask->Value(x, y)) {
				long double value = image->Value(x, y);
				size_t index = (size_t) ((value * stretch - min) / binsize);
				if(index < binCount)
					valuesOutput[index] += 1;
			}
		}
	}
	if(factor != 1.0) {
		for(size_t i=0;i<binCount;++i) {
			valuesOutput[i] = (size_t) (factor * valuesOutput[i]);
		}
	}
}

void RFIPlots::MakeDistPlot(Plot &plot, Image2DCPtr image, Mask2DCPtr mask)
{
	std::vector<size_t> valuesOutput;
	std::vector<long double> binsOutput;
	plot.SetXAxisText("Flux (Jy)");
	plot.SetYAxisText("Occurences");

	long double mean, stddev;
	long double min = image->GetMinimum();
	long double max = image->GetMaximum();
	ThresholdTools::WinsorizedMeanAndStdDev(image, mean, stddev);
	if(min < mean-3.0L*stddev)
		min = mean-3.0L*stddev;
	if(max > mean+3.0L*stddev)
		max = mean+3.0L*stddev;

	Bin(image, mask, valuesOutput, binsOutput, 40, min, max);
	for(unsigned i=0;i<valuesOutput.size();++i)
		plot.PushDataPoint(binsOutput[i], valuesOutput[i]);
}

void RFIPlots::MakePowerSpectrumPlot(class Plot &plot, Image2DCPtr image, Mask2DCPtr mask)
{
	plot.SetXAxisText("Channel");
	plot.SetYAxisText("Flux density (Jy)");
	plot.SetLogScale(false, true, false);
	plot.SetXRange(0.0, image->Height()-1);

	long double min = 1e100, max = 0.0;

	for(size_t y=0;y<image->Height();++y) {
		long double sum = 0.0L;
		size_t count = 0;
		for(size_t x=0;x<image->Width();++x) {
			if(!mask->Value(x, y) && std::isnormal(image->Value(x, y))) {
				sum += image->Value(x, y);
				++count;
			}
		}
		if(count > 0)
		{
			long double v = sum/count;
			if(v < min) min = v;
			if(v > max) max = v;
			plot.PushDataPoint(y, v);
		}
	}
	plot.SetYRange(min * 0.9, max / 0.9);
}

void RFIPlots::MakePowerTimePlot(class Plot &plot, Image2DCPtr image, Mask2DCPtr mask)
{
	plot.SetXAxisText("Time");
	plot.SetYAxisText("Flux density (Jy)");
	plot.SetLogScale(false, true, false);
	plot.SetXRange(0.0, image->Width()-1);

	size_t binSize = (size_t) ceil(image->Width() / 256.0L);

	for(size_t x=0;x<image->Width();x += binSize) {
		long double sum = 0.0L;
		size_t count = 0;
		for(size_t binx=0;binx<binSize;++binx) {
			for(size_t y=0;y<image->Height();++y) {
				if(!mask->Value(x + binx, y) && std::isnormal(image->Value(x + binx, y))) {
					sum += image->Value(x + binx, y);
					++count;
				}
			}
		}
		plot.PushDataPoint(x + binSize / 2, sum / count);
	}
}

void RFIPlots::MakeComplexPlanePlot(class Plot &plot, const TimeFrequencyData &data, size_t xStart, size_t length, size_t y, size_t yAvgSize, Mask2DCPtr mask, bool realVersusImaginary, bool drawImaginary)
{

	if(realVersusImaginary)
	{
		plot.SetXAxisText("real");
		plot.SetYAxisText("imaginary");
	} else {
		plot.SetXRange(xStart, xStart+length-1);
		plot.SetXAxisText("time");
		plot.SetYAxisText("real/imaginary");
	}

	Image2DCPtr real = data.GetRealPart();
	Image2DCPtr imaginary = data.GetImaginaryPart();

	for(size_t x=xStart;x<xStart + length;++x)
	{
		long double r = 0.0L, i = 0.0L;
		for(size_t yi=y;yi<yAvgSize+y;++yi)
		{
			if(!mask->Value(x, yi) && std::isfinite(real->Value(x,yi)) && std::isfinite(imaginary->Value(x,yi)))
			{
				r += real->Value(x, yi);
				i += imaginary->Value(x, yi);
			}
		}
		if(realVersusImaginary)
			plot.PushDataPoint(r, i);
		else if(drawImaginary)
			plot.PushDataPoint(x, i);
		else
			plot.PushDataPoint(x, r);
	}
}

void RFIPlots::MakeFittedComplexPlot(class Plot &plot, const TimeFrequencyData &data, size_t xStart, size_t length, size_t y, size_t yAvgSize, Mask2DCPtr mask, long double frequency, bool realVersusImaginary, bool drawImaginary)
{
	if(realVersusImaginary)
	{
		plot.SetXAxisText("real");
		plot.SetYAxisText("imaginary");
	} else {
		plot.SetXRange(xStart, xStart+length-1);
		plot.SetXAxisText("time");
		plot.SetYAxisText("real/imaginary flux density");
	}
	Image2DCPtr real = data.GetRealPart();
	Image2DCPtr imaginary = data.GetImaginaryPart();

	long double *xReal = new long double[length];
	long double *xImag = new long double[length];
	long double *t = new long double[length];
	size_t dataIndex = 0;

	for(size_t x=xStart;x<xStart + length;++x)
	{
		long double r = 0.0L, i = 0.0L;
		size_t count = 0;
		for(size_t yi=y;yi<yAvgSize+y;++yi)
		{
			if(!mask->Value(x, yi) && std::isfinite(real->Value(x,yi)) && std::isfinite(imaginary->Value(x,yi)) )
			{
				r += real->Value(x, yi);
				i += imaginary->Value(x, yi);
				++count;
			}
		}
		if(count > 0)
		{
			t[dataIndex] = x;
			xReal[dataIndex] = r;
			xImag[dataIndex] = i;
			++dataIndex;
		}
	}
	if(dataIndex != length)
		std::cout << "Warning: " << (length-dataIndex) << " time points were removed." << std::endl; 
	SinusFitter fitter;
	long double
		realPhase, realAmplitude, realMean,
		imagPhase, imagAmplitude, imagMean;
	const long double twopi = 2.0L*M_PI;

	//fitter.FindPhaseAndAmplitude(realPhase, realAmplitude, xReal, t, dataIndex, frequency*twopi);
	//fitter.FindPhaseAndAmplitude(imagPhase, imagAmplitude, xImag, t, dataIndex, frequency*twopi);
	//realMean = fitter.FindMean(realPhase, realAmplitude, xReal, t, dataIndex, frequency*twopi);
	//imagMean = fitter.FindMean(imagPhase, imagAmplitude, xImag, t, dataIndex, frequency*twopi);
	fitter.FindPhaseAndAmplitudeComplex(realPhase, realAmplitude, xReal, xImag, t, dataIndex, frequency*twopi);
	imagPhase = realPhase + 0.5L*M_PIl;
	imagAmplitude = realAmplitude;
	realMean = fitter.FindMean(realPhase, realAmplitude, xReal, t, dataIndex, frequency*twopi);
	imagMean = fitter.FindMean(imagPhase, imagAmplitude, xImag, t, dataIndex, frequency*twopi);

	for(size_t x=xStart;x<xStart + length;++x)
	{
		if(realVersusImaginary)
			plot.PushDataPoint(
				cosl(frequency*2.0L*M_PIl*(long double) x + realPhase) * realAmplitude + realMean,
				cosl(frequency*2.0L*M_PIl*(long double) x + imagPhase) * imagAmplitude + imagMean);
		else if(drawImaginary)
			plot.PushDataPoint(x,
				cosl(frequency*2.0L*M_PIl*(long double) x + imagPhase) * imagAmplitude + imagMean);
		else
			plot.PushDataPoint(x,
				cosl(frequency*2.0L*M_PIl*(long double) x + realPhase) * realAmplitude + realMean);
	}

	delete t;
	delete xReal;
	delete xImag;
}

void RFIPlots::MakeScatterPlot(class MultiPlot &plot, size_t plotIndex, Image2DCPtr image, Mask2DCPtr mask)
{
	plot.SetXAxisText("Time");
	plot.SetYAxisText("Flux density (Jy)");
	plot.SetLogScale(false, false, false);
	plot.SetXRange(0.0, image->Width()-1);

	for(size_t x=0;x<image->Width();++x) {
		size_t count = 0;
		num_t sum = 0.0;
		for(size_t y=0;y<image->Height();++y) {
			if(!mask->Value(x, y) && std::isnormal(image->Value(x, y))) {
				sum += image->Value(x, y);
				++count;
			}
		}
		if(count > 0)
			plot.AddPoint(plotIndex, x, sum / count);
	}
}

void RFIPlots::MakeScatterPlot(class MultiPlot &plot, size_t plotIndex, SampleRowCPtr row)
{
	plot.SetXRange(0.0, row->Size()-1);

	for(size_t x=0;x<row->Size();++x) {
		if(!row->ValueIsMissing(x))
			plot.AddPoint(plotIndex, x, row->Value(x));
	}
}

void RFIPlots::MakeScatterPlot(class MultiPlot &plot, const TimeFrequencyData &data)
{
	switch(data.Polarisation())
	{
		case DipolePolarisation:
		{
			TimeFrequencyData
				*xx = data.CreateTFData(XXPolarisation),
				*xy = data.CreateTFData(XYPolarisation),
				*yx = data.CreateTFData(YXPolarisation),
				*yy = data.CreateTFData(YYPolarisation);
			MakeScatterPlot(plot, 0, xx->GetSingleImage(), xx->GetSingleMask());
			MakeScatterPlot(plot, 1, xy->GetSingleImage(), xy->GetSingleMask());
			MakeScatterPlot(plot, 2, yx->GetSingleImage(), yx->GetSingleMask());
			MakeScatterPlot(plot, 3, yy->GetSingleImage(), yy->GetSingleMask());
			delete xx;
			delete xy;
			delete yx;
			delete yy;
			plot.SetLegend(0, "XX");
			plot.SetLegend(1, "XY");
			plot.SetLegend(2, "YX");
			plot.SetLegend(3, "YY");
			break;
		}
		case AutoDipolePolarisation:
		{
			TimeFrequencyData
				*xx = data.CreateTFData(XXPolarisation),
				*yy = data.CreateTFData(YYPolarisation);
			MakeScatterPlot(plot, 0, xx->GetSingleImage(), xx->GetSingleMask());
			MakeScatterPlot(plot, 1, yy->GetSingleImage(), yy->GetSingleMask());
			plot.SetLegend(0, "XX");
			plot.SetLegend(1, "YY");
			delete xx;
			delete yy;
			break;
		}
		case CrossDipolePolarisation:
		{
			TimeFrequencyData
				*xy = data.CreateTFData(XYPolarisation),
				*yx = data.CreateTFData(YXPolarisation);
			MakeScatterPlot(plot, 0, xy->GetSingleImage(), xy->GetSingleMask());
			MakeScatterPlot(plot, 1, yx->GetSingleImage(), yx->GetSingleMask());
			plot.SetLegend(0, "XY");
			plot.SetLegend(1, "XY");
			delete xy;
			delete yx;
			break;
		}
		case SinglePolarisation:
		case StokesIPolarisation:
		case XXPolarisation:
		case XYPolarisation:
		case YXPolarisation:
		case YYPolarisation:
			MakeScatterPlot(plot, 0, data.GetSingleImage(), data.GetSingleMask());
			plot.SetLegend(0, data.Description());
		break;
	}
}

void RFIPlots::MakeQualityPlot(class Plot &plot, const TimeFrequencyData &original, const TimeFrequencyData &model, size_t partCount)
{
	plot.SetXRange(0, model.ImageWidth()-1);
	plot.StartLine();
	Image2DCPtr originalImg = original.GetSingleImage();
	Image2DCPtr modelImg = model.GetSingleImage();
	Mask2DCPtr mask = original.GetSingleMask();
	for(unsigned p=0;p<=partCount;++p)
	{
		unsigned xStart = model.ImageWidth() * p / partCount;
		unsigned xEnd = model.ImageWidth() * (p+1) / partCount;
		double quality = RFIStatistics::DataQuality(originalImg, modelImg, mask, xStart, xEnd);
		plot.PushDataPoint((xStart+xEnd)/2, quality);
	}
}

void RFIPlots::MakeRMSSpectrumPlot(class Plot &plot, Image2DCPtr image, Mask2DCPtr mask)
{
	plot.SetXAxisText("Channel");
	plot.SetYAxisText("RMS in flux density (Jy)");
	plot.SetLogScale(false, true, false);
	plot.SetXRange(0.0, image->Height()-1);

	long double min = 1e100, max = 0.0;

	for(size_t y=0;y<image->Height();++y) {
		long double sum = 0.0L;
		size_t count = 0;
		for(size_t x=0;x<image->Width();++x) {
			if(!mask->Value(x, y) && std::isnormal(image->Value(x, y))) {
				sum += image->Value(x, y) * image->Value(x, y);
				++count;
			}
		}
		if(count > 0)
		{
			long double v = sqrtl(sum/count);

			if(v < min) min = v;
			if(v > max) max = v;
			plot.PushDataPoint(y, v);
		}
	}
	plot.SetYRange(min, max);
}

void RFIPlots::MakeSNRSpectrumPlot(class Plot &plot, Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask)
{
	plot.SetXAxisText("Channel");
	plot.SetYAxisText("RMS in flux density (Jy)");
	plot.SetLogScale(false, true, false);
	plot.SetXRange(0.0, image->Height()-1);

	long double min = 1e100, max = 0.0;

	for(size_t y=0;y<image->Height();++y) {
		num_t v = RFIStatistics::FrequencySNR(image, model, mask, y);

		if(v < min) min = v;
		if(v > max) max = v;
		plot.PushDataPoint(y, v);
	}
	plot.SetYRange(min, max);
}
