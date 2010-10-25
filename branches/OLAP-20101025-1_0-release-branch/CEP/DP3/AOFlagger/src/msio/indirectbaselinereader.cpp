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
#include <AOFlagger/msio/indirectbaselinereader.h>

#include <fstream>
#include <set>
#include <stdexcept>

#include "boost/filesystem.hpp"

#include <AOFlagger/msio/arraycolumniterator.h>
#include <AOFlagger/msio/scalarcolumniterator.h>
#include <AOFlagger/msio/timefrequencydata.h>

IndirectBaselineReader::IndirectBaselineReader(const std::string &msFile) : BaselineReader(msFile), _directReader(msFile), _msIsReordered(false), _maxMemoryUse(1024*1024*1024)
{
}

IndirectBaselineReader::~IndirectBaselineReader()
{
	removeTemporaryFiles();
}

void IndirectBaselineReader::PerformReadRequests()
{
	initialize();

	if(!_msIsReordered) reorderMS();

	_results.clear();
	std::cout << "Performing " << _readRequests.size() << " read requests..." << std::endl;
	for(size_t i=0;i<_readRequests.size();++i)
	{
		const ReadRequest request = _readRequests[i];
		_results.push_back(Result());
		const size_t width = ObservationTimes().size();
		for(size_t p=0;p<PolarizationCount();++p)
		{
			if(ReadData()) {
				_results[i]._realImages.push_back(Image2D::CreateEmptyImagePtr(width, FrequencyCount()));
				_results[i]._imaginaryImages.push_back(Image2D::CreateEmptyImagePtr(width, FrequencyCount()));
			}
			if(ReadFlags()) {
				// The flags should be initialized to true, as a baseline might
				// miss some time scans that other baselines do have, and these
				// should be flagged.
				_results[i]._flags.push_back(Mask2D::CreateSetMaskPtr<true>(width, FrequencyCount()));
			}
		}
		_results[i]._uvw.resize(width);

		std::stringstream dataFilename;
		dataFilename << "data-" << request.antenna1 << "x" << request.antenna2 << ".tmp";
		std::ifstream dataFile(dataFilename.str().c_str(), std::ifstream::binary);

		std::stringstream flagFilename;
		flagFilename << "flag-" << request.antenna1 << "x" << request.antenna2 << ".tmp";
		std::ifstream flagFile(flagFilename.str().c_str(), std::ifstream::binary);

		const size_t bufferSize = FrequencyCount() * PolarizationCount();
		for(size_t x=0;x<width;++x)
		{
			float dataBuffer[bufferSize*2];
			bool flagBuffer[bufferSize];
			dataFile.read((char *) dataBuffer, bufferSize * sizeof(float) * 2);
			size_t dataBufferPtr = 0;
			flagFile.read((char *) flagBuffer, bufferSize * sizeof(bool));
			size_t flagBufferPtr = 0;
			for(size_t f=0;f<FrequencyCount();++f) {
				for(size_t p=0;p<PolarizationCount();++p)
				{
					_results[i]._realImages[p]->SetValue(x, f, dataBuffer[dataBufferPtr]);
					++dataBufferPtr;
					_results[i]._imaginaryImages[p]->SetValue(x, f, dataBuffer[dataBufferPtr]);
					++dataBufferPtr;
					_results[i]._flags[p]->SetValue(x, f, flagBuffer[flagBufferPtr]);
					++flagBufferPtr;
				}
			}
		}
	}
	std::cout << "Done reading" << std::endl;

	_readRequests.clear();
}

void IndirectBaselineReader::PerformWriteRequests()
{
	for(size_t i=0;i!=_writeRequests.size();++i)
	{
		WriteRequest request = _writeRequests[i];
		_directReader.AddWriteTask(request.flags, request.antenna1, request.antenna2, request.spectralWindow, request.startIndex, request.endIndex, request.leftBorder, request.rightBorder);
	}
	_writeRequests.clear();
	_directReader.PerformWriteRequests();
}

void IndirectBaselineReader::reorderMS()
{
	casa::Table &table = *Table();

	casa::ROScalarColumn<double> timeColumn(*Table(), "TIME");
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");

	int rowCount = table.nrow();
	if(rowCount == 0)
		throw std::runtime_error("Measurement set is empty (zero rows)");

	casa::ROArrayColumn<casa::Complex> *dataColumn = CreateDataColumn(DataKind(), table);

	std::vector<std::pair<size_t,size_t> > baselines;
	Set().GetBaselines(baselines);
	size_t
		antennaCount = Set().AntennaCount(),
		frequencyCount = FrequencyCount(),
		polarizationCount = PolarizationCount();

	size_t bufferSize = _maxMemoryUse / (baselines.size() * frequencyCount * polarizationCount * (sizeof(float) * 2 + sizeof(bool)));

	std::vector<std::vector<float *> > dataBuffers;
	std::vector<std::vector<bool *> > flagBuffers;
	std::vector<std::vector<std::ofstream *> > dataFiles;
	std::vector<std::vector<std::ofstream *> > flagFiles;
	for(size_t i=0;i<antennaCount;++i) {
		dataBuffers.push_back(std::vector<float *>());
		dataFiles.push_back(std::vector<std::ofstream *>());
		flagBuffers.push_back(std::vector<bool *>());
		flagFiles.push_back(std::vector<std::ofstream *>());
		for(size_t j=0;j<antennaCount;++j) {
			dataBuffers[i].push_back(0);
			dataFiles[i].push_back(0);
			flagBuffers[i].push_back(0);
			flagFiles[i].push_back(0);
		}
	}

	std::cout << "Requesting " << (sizeof(float)*2+sizeof(bool)) << " x " << baselines.size() << " x " << bufferSize << " x " << polarizationCount << " x " << frequencyCount << " bytes of data" << std::endl;
	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		std::stringstream dataFilename;
		dataFilename << "data-" << i->first << "x" << i->second << ".tmp";
		std::stringstream flagFilename;
		flagFilename << "flag-" << i->first << "x" << i->second << ".tmp";
		dataBuffers[i->first][i->second] = new float[2 * bufferSize * frequencyCount * polarizationCount];
		dataFiles[i->first][i->second] = new std::ofstream(dataFilename.str().c_str(), std::ofstream::binary);
		flagBuffers[i->first][i->second] = new bool[bufferSize * frequencyCount * polarizationCount];
		flagFiles[i->first][i->second] = new std::ofstream(flagFilename.str().c_str(), std::ofstream::binary);
	}

	size_t
		prevTimeIndex = (size_t) (-1),
		timeIndex = 0,
		currentBufferBlockPtr = (size_t) (-1);
	double prevTime = 0.0;

	std::cout << "R" << std::flush;
	for(int rowIndex = 0;rowIndex < rowCount;++rowIndex)
	{
		double time = timeColumn(rowIndex);
		if(time != prevTime)
		{
			timeIndex = ObservationTimes().find(time)->second;
			if(timeIndex != prevTimeIndex+1)
			{
				std::stringstream s;
				s << "Error: time step " << prevTimeIndex << " is followed by time step " << timeIndex;
				throw std::runtime_error(s.str());
			}
			prevTime = time;
			prevTimeIndex = timeIndex;
			++currentBufferBlockPtr;

			if(currentBufferBlockPtr >= bufferSize)
			{
				// buffer is full, flush
				std::cout << "W" << std::flush;
				for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
				{
					const size_t sampleCount = bufferSize * frequencyCount * polarizationCount;
					float *dataBuffer = dataBuffers[i->first][i->second];
					bool *flagBuffer = flagBuffers[i->first][i->second];
					std::ofstream *dataFile = dataFiles[i->first][i->second];
					std::ofstream *flagFile = flagFiles[i->first][i->second];
					dataFile->write((char *) dataBuffer, sampleCount * 2 * sizeof(float));
					flagFile->write((char *) flagBuffer, sampleCount * sizeof(bool));
				}

				std::cout << "R" << std::flush;
				currentBufferBlockPtr = 0;
			}
		}
		
		casa::Array<casa::Complex> data = (*dataColumn)(rowIndex);
		casa::Array<casa::Complex>::const_iterator dataI=data.begin();
		casa::Array<bool> flag = flagColumn(rowIndex);
		casa::Array<bool>::const_iterator flagI=flag.begin();
		size_t dataBufferPtr = currentBufferBlockPtr*2*frequencyCount*polarizationCount;
		float *dataBuffer = dataBuffers[antenna1Column(rowIndex)][antenna2Column(rowIndex)];
		size_t flagBufferPtr = currentBufferBlockPtr*frequencyCount*polarizationCount;
		bool *flagBuffer = flagBuffers[antenna1Column(rowIndex)][antenna2Column(rowIndex)];
		for(size_t f=0;f<frequencyCount;++f) {
			for(size_t p=0;p<polarizationCount;++p) {
				dataBuffer[dataBufferPtr] = dataI->real();
				++dataBufferPtr;
				dataBuffer[dataBufferPtr] = dataI->imag();
				++dataI;
				++dataBufferPtr;

				flagBuffer[flagBufferPtr] = *flagI;
				++flagBufferPtr;
				++flagI;
			}
		}
	}
	
	// flush half-full buffer
	std::cout << "W" << std::flush;
	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		const size_t sampleCount = currentBufferBlockPtr * frequencyCount * polarizationCount;
		float *dataBuffer = dataBuffers[i->first][i->second];
		bool *flagBuffer = flagBuffers[i->first][i->second];
		std::ofstream *dataFile = dataFiles[i->first][i->second];
		std::ofstream *flagFile = flagFiles[i->first][i->second];
		dataFile->write((char *) dataBuffer, sampleCount * 2 * sizeof(float));
		flagFile->write((char *) flagBuffer, sampleCount * sizeof(bool));
	}

	std::cout << "\nFreeing the data" << std::endl;
	for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
	{
		delete[] dataBuffers[i->first][i->second];
		delete[] flagBuffers[i->first][i->second];
		delete dataFiles[i->first][i->second];
	}

	delete dataColumn;

	clearTableCaches();

	std::cout << "Done reordering data set" << std::endl;
	_msIsReordered = true;
}

void IndirectBaselineReader::removeTemporaryFiles()
{
	if(_msIsReordered)
	{
		std::vector<std::pair<size_t,size_t> > baselines;
		Set().GetBaselines(baselines);
		for(std::vector<std::pair<size_t,size_t> >::const_iterator i=baselines.begin();i<baselines.end();++i)
		{
			std::stringstream dataFilename;
			dataFilename << "data-" << i->first << "x" << i->second << ".tmp";
			boost::filesystem::remove(dataFilename.str());

			std::stringstream flagFilename;
			flagFilename << "flag-" << i->first << "x" << i->second << ".tmp";
			boost::filesystem::remove(flagFilename.str());
		}
		std::cout << "Temporary files removed." << std::endl;
	}
	_msIsReordered = false;
}

