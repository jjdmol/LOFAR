#include <AOFlagger/msio/baselinematrixloader.h>

#include <stdexcept>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableIter.h>

#include <AOFlagger/msio/arraycolumniterator.h>
#include <AOFlagger/msio/scalarcolumniterator.h>

BaselineMatrixLoader::BaselineMatrixLoader(MeasurementSet &measurementSet)
	: _sortedTable(0), _measurementSet(measurementSet), _timeIndexCount(0)
{
	casa::Table *rawTable = _measurementSet.OpenTable(MeasurementSet::MainTable);
	casa::Block<casa::String> names(4);
	names[0] = "DATA_DESC_ID";
	names[1] = "TIME";
	names[2] = "ANTENNA1";
	names[3] = "ANTENNA2";
	_sortedTable = new casa::Table(rawTable->sort(names));
	delete rawTable;

	casa::Block<casa::String> selectionNames(2);
	selectionNames[0] = "DATA_DESC_ID";
	selectionNames[1] = "TIME";
	casa::TableIterator iter(*_sortedTable, selectionNames, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
	while(!iter.pastEnd())
	{
		iter.next();
		++_timeIndexCount;
	}
}

BaselineMatrixLoader::~BaselineMatrixLoader()
{
	if(_sortedTable != 0)
		delete _sortedTable;
}

TimeFrequencyData BaselineMatrixLoader::Load(size_t timeIndex)
{
	casa::Block<casa::String> selectionNames(2);
	selectionNames[0] = "DATA_DESC_ID";
	selectionNames[1] = "TIME";
	casa::TableIterator iter(*_sortedTable, selectionNames, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
	while(!iter.pastEnd() && timeIndex > 0)
	{
		iter.next();
		--timeIndex;
	}
	if(iter.pastEnd())
	{
		throw std::runtime_error("Time index not found");
	}

	casa::Table table = iter.table();
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");

	ScalarColumnIterator<int> antenna1Iter = ScalarColumnIterator<int>::First(antenna1Column);
	ScalarColumnIterator<int> antenna2Iter = ScalarColumnIterator<int>::First(antenna2Column);
	ROArrayColumnIterator<casa::Complex> dataIter = 
		ROArrayColumnIterator<casa::Complex>::First(dataColumn);
	ROArrayColumnIterator<bool> flagIter = 
		ROArrayColumnIterator<bool>::First(flagColumn);

	// Find highest antenna index
	int nrAntenna = 0;
	for(size_t i=0;i<table.nrow();++i)
	{
		int
			a1 = *antenna1Iter,
			a2 = *antenna2Iter;

		if(a1 > nrAntenna)
			nrAntenna = a1;
		if(a2 > nrAntenna)
			nrAntenna = a2;

		++antenna1Iter;
		++antenna2Iter;
	}
	++nrAntenna;

	antenna1Iter = ScalarColumnIterator<int>::First(antenna1Column);
	antenna2Iter = ScalarColumnIterator<int>::First(antenna2Column);

	Image2DPtr
		xxRImage = Image2D::CreateEmptyImagePtr(nrAntenna, nrAntenna),
		xxIImage = Image2D::CreateEmptyImagePtr(nrAntenna, nrAntenna),
		xyRImage = Image2D::CreateEmptyImagePtr(nrAntenna, nrAntenna),
		xyIImage = Image2D::CreateEmptyImagePtr(nrAntenna, nrAntenna),
		yxRImage = Image2D::CreateEmptyImagePtr(nrAntenna, nrAntenna),
		yxIImage = Image2D::CreateEmptyImagePtr(nrAntenna, nrAntenna),
		yyRImage = Image2D::CreateEmptyImagePtr(nrAntenna, nrAntenna),
		yyIImage = Image2D::CreateEmptyImagePtr(nrAntenna, nrAntenna);
	Mask2DPtr
		xxMask = Mask2D::CreateUnsetMaskPtr(nrAntenna, nrAntenna),
		xyMask = Mask2D::CreateUnsetMaskPtr(nrAntenna, nrAntenna),
		yxMask = Mask2D::CreateUnsetMaskPtr(nrAntenna, nrAntenna),
		yyMask = Mask2D::CreateUnsetMaskPtr(nrAntenna, nrAntenna);

	size_t frequencyCount = _measurementSet.FrequencyCount();

	for(size_t i=0;i<table.nrow();++i)
	{
		int
			a1 = *antenna1Iter,
			a2 = *antenna2Iter;
		casa::Array<casa::Complex> data =
			*dataIter;

		casa::Array<casa::Complex>::const_iterator i = data.begin();
		num_t
			xxr = 0.0, xxi = 0.0, xyr = 0.0, xyi = 0.0, yxr = 0.0, yxi = 0.0, yyr = 0.0, yyi = 0.0;
		size_t
			xxc = 0, xyc = 0, yxc = 0, yyc = 0;
		for(size_t f=0;f<(size_t) frequencyCount;++f) {
			const casa::Complex &xx = *i;
			++i;
			const casa::Complex &xy = *i;
			++i;
			const casa::Complex &yx = *i;
			++i;
			const casa::Complex &yy = *i;
			++i;
			if(std::isfinite(xxr) && std::isfinite(xxi))
			{
				xxr += xx.real();
				xxi += xx.imag();
				++xxc;
			}
			if(std::isfinite(xyr) && std::isfinite(xyi))
			{
				xyr += xy.real();
				xyi += xy.imag();
				++xyc;
			}
			if(std::isfinite(yxr) && std::isfinite(yxi))
			{
				yxr += yx.real();
				yxi += yx.imag();
				++yxc;
			}
			if(std::isfinite(yyr) && std::isfinite(yyi))
			{
				yyr += yy.real();
				yyi += yy.imag();
				++yyc;
			}
		}

		xxRImage->SetValue(a1, a2, xxr);
		xxIImage->SetValue(a1, a2, xxi);
		xyRImage->SetValue(a1, a2, xyr);
		xyIImage->SetValue(a1, a2, xyi);
		yxRImage->SetValue(a1, a2, yxr);
		yxIImage->SetValue(a1, a2, yxi);
		yyRImage->SetValue(a1, a2, yyr);
		yyIImage->SetValue(a1, a2, yyi);

		xxMask->SetValue(a1, a2, xxc == 0);
		xyMask->SetValue(a1, a2, xyc == 0);
		yxMask->SetValue(a1, a2, yxc == 0);
		yyMask->SetValue(a1, a2, yyc == 0);

		if(a1 != a2)
		{
			xxRImage->SetValue(a2, a1, xxr);
			xxIImage->SetValue(a2, a1, -xxi);
			xyRImage->SetValue(a2, a1, xyr);
			xyIImage->SetValue(a2, a1, -xyi);
			yxRImage->SetValue(a2, a1, yxr);
			yxIImage->SetValue(a2, a1, -yxi);
			yyRImage->SetValue(a2, a1, yyr);
			yyIImage->SetValue(a2, a1, -yyi);

			xxMask->SetValue(a2, a1, xxc == 0);
			xyMask->SetValue(a2, a1, xyc == 0);
			yxMask->SetValue(a2, a1, yxc == 0);
			yyMask->SetValue(a2, a1, yyc == 0);
		}

		++antenna1Iter;
		++antenna2Iter;
		++dataIter;
	}
	TimeFrequencyData data(xxRImage, xxIImage, xyRImage, xyIImage, yxRImage, yxIImage, yyRImage, yyIImage);
	data.SetIndividualPolarisationMasks(xxMask, xyMask, yxMask, yyMask);
	return data;
}
