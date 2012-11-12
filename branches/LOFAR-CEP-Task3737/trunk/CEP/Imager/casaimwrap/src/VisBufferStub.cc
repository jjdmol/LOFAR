//# VisBufferStub.cc: Hacked VisBuffer implementation that can be used to pass
//# visibility data between Python and CASA FTMachines.
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <casaimwrap/VisBufferStub.h>

namespace LOFAR
{
namespace casaimwrap
{
using namespace casa;

VisBufferStub::VisBufferStub(const MeasurementSet &ms)
    :   VisBuffer(),
        itsROMSColumns(ms),
        itsNewMS(true),
        itsSpectralWindow(0),
        itsNRow(0)
{
    const unsigned int idDataDescription = 0;
    const unsigned int idField = 0;

    // Read polarization id and spectral window id.
    ROMSDataDescColumns desc(ms.dataDescription());
    AlwaysAssert(desc.nrow() > idDataDescription, AipsError);
    AlwaysAssert(!desc.flagRow()(idDataDescription), AipsError);

    const unsigned int idPolarization =
        desc.polarizationId()(idDataDescription);
    const unsigned int idWindow =
        desc.spectralWindowId()(idDataDescription);

    // Get spectral information.
    ROMSSpWindowColumns window(ms.spectralWindow());
    AlwaysAssert(window.nrow() > idWindow, AipsError);
    AlwaysAssert(!window.flagRow()(idWindow), AipsError);

    itsNChannel = window.numChan()(idWindow);
    itsFrequency = window.chanFreq()(idWindow);
    AlwaysAssert(itsFrequency.size() == itsNChannel, AipsError);

    // Initialize correlation axis.
    ROMSPolarizationColumns polarization(ms.polarization());
    AlwaysAssert(polarization.nrow() > idPolarization, AipsError);
    AlwaysAssert(!polarization.flagRow()(idPolarization), AipsError);

    itsCorrType = polarization.corrType()(idPolarization);
    itsNCorr = itsCorrType.size();

    ROMSFieldColumns field(ms.field());
    AlwaysAssert(field.nrow() > idField, AipsError);
    AlwaysAssert(!field.flagRow()(idField), AipsError);

    itsPhaseCenter = field.phaseDirMeas(idField);
}

void VisBufferStub::setChunk(const Vector<Int> &antenna1,
    const Vector<Int> &antenna2,
    const Matrix<Double> &uvw,
    const Vector<Double> &time,
    const Vector<Double> &timeCentroid,
    const Vector<Bool> &flagRow,
    const Matrix<Float> &weight,
    const Cube<Bool> &flag,
    const Cube<Complex> &data,
    Bool newMS)
{
    itsAntenna1.reference(antenna1);
    itsAntenna2.reference(antenna2);
    itsTime.reference(time);
    itsTimeCentroid.reference(timeCentroid);
    itsFlagRow.reference(flagRow);
    itsWeight.reference(weight);
    itsFlag.reference(flag);
    itsData.reference(data);
    itsNewMS = newMS;

    itsNRow = itsTime.size();

    // Convert UVW coordinates to the required type.
    itsUVW.resize(itsNRow);
    for(Int i = 0; i < itsNRow; ++i)
    {
        itsUVW(i)(0) = uvw(0, i);
        itsUVW(i)(1) = uvw(1, i);
        itsUVW(i)(2) = uvw(2, i);
    }

    // Compute imaging weights.
    VisImagingWeight imw("natural");

    Matrix<Bool> tmp_flag;
    tmp_flag.resize(itsNChannel, itsNRow);
    for(Int row = 0; row < itsNRow; row++)
    {
        for(Int ch = 0; ch < itsNChannel; ch++)
        {
            tmp_flag(ch,row) = itsFlag(0, ch, row);
            for(Int cr = 1; cr < itsNCorr; cr++)
            {
                tmp_flag(ch,row) |= itsFlag(cr, ch, row);
            }
        }
    }

    // Take average of parallel hand polarizations for now.
    // Later convert weight() to return full polarization dependence.
    Vector<Float> tmp_weight;
    tmp_weight.resize(itsNRow);
    tmp_weight = itsWeight.row(0);
    tmp_weight += itsWeight.row(itsNCorr - 1);
    tmp_weight /= 2.0f;

    itsImagingWeight.resize(itsNChannel, itsNRow);
    imw.weightNatural(itsImagingWeight, tmp_flag, tmp_weight);
}

void VisBufferStub::setChunk(const Vector<Int> &antenna1,
    const Vector<Int> &antenna2,
    const Matrix<Double> &uvw,
    const Vector<Double> &time,
    const Vector<Double> &timeCentroid,
    const Vector<Bool> &flagRow,
    const Matrix<Float> &weight,
    const Cube<Bool> &flag,
    Bool newMS)
{
    setChunk(antenna1, antenna2, uvw, time, timeCentroid, flagRow, weight, flag,
        Cube<Complex>(), newMS);
    itsData.resize(itsFlag.shape());
    itsData.set(0.0);
}

VisBuffer &VisBufferStub::assign(const VisBuffer & vb, Bool copy)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::attachToVisIter(ROVisibilityIterator & iter)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::detachFromVisIter()
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::invalidate()
{
    AlwaysAssert(false, AipsError);
}

Int &VisBufferStub::nCorr()
{
    return itsNCorr;
}

Int VisBufferStub::nCorr() const
{
    return itsNCorr;
}

Int &VisBufferStub::nChannel()
{
    return itsNChannel;
}

Int VisBufferStub::nChannel() const
{
    return itsNChannel;
}

Vector<Int> &VisBufferStub::channel()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Int> &VisBufferStub::channel() const
{
    AlwaysAssert(false, AipsError);
}

Int &VisBufferStub::nRow()
{
    return itsNRow;
}

Int VisBufferStub::nRow() const
{
    return itsNRow;
}

Vector<Int> &VisBufferStub::antenna1()
{
    return itsAntenna1;
}

const Vector<Int> &VisBufferStub::antenna1() const
{
    return itsAntenna1;
}

Vector<Int> &VisBufferStub::antenna2()
{
    return itsAntenna2;
}

const Vector<Int> &VisBufferStub::antenna2() const
{
    return itsAntenna2;
}

Vector<Int> &VisBufferStub::feed1()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Int> &VisBufferStub::feed1() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Int> &VisBufferStub::feed2()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Int> &VisBufferStub::feed2() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Float> &VisBufferStub::feed1_pa()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Float> &VisBufferStub::feed1_pa() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Float> &VisBufferStub::feed2_pa()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Float> &VisBufferStub::feed2_pa() const
{
    AlwaysAssert(false, AipsError);
}

Vector<SquareMatrix<Complex, 2> > &VisBufferStub::CJones()
{
    AlwaysAssert(false, AipsError);
}

const Vector<SquareMatrix<Complex, 2> > &VisBufferStub::CJones() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Float> VisBufferStub::feed_pa(Double time) const
{
    AlwaysAssert(false, AipsError);
}

Vector<MDirection> &VisBufferStub::direction1()
{
    AlwaysAssert(false, AipsError);
}

const Vector<MDirection> &VisBufferStub::direction1() const
{
    AlwaysAssert(false, AipsError);
}

Vector<MDirection> &VisBufferStub::direction2()
{
    AlwaysAssert(false, AipsError);
}

const Vector<MDirection> &VisBufferStub::direction2() const
{
    AlwaysAssert(false, AipsError);
}

Float VisBufferStub::parang0(Double time) const
{
    AlwaysAssert(false, AipsError);
}

Vector<Float> VisBufferStub::parang(Double time) const
{
    AlwaysAssert(false, AipsError);
}

MDirection VisBufferStub::azel0(Double time) const
{
    AlwaysAssert(false, AipsError);
}

Vector<Double> &VisBufferStub::azel0Vec(Double time, Vector<Double>& azelVec)
    const
{
    AlwaysAssert(false, AipsError);
}

Vector<MDirection> VisBufferStub::azel(Double time) const
{
    AlwaysAssert(false, AipsError);
}

Matrix<Double> &VisBufferStub::azelMat(Double time, Matrix<Double>& azelMat)
    const
{
    AlwaysAssert(false, AipsError);
}

Double VisBufferStub::hourang(Double time) const
{
    AlwaysAssert(false, AipsError);
}

Int VisBufferStub::fieldId() const
{
    return 0;
}

Int VisBufferStub::arrayId() const
{
    return 0;
}

Matrix<Bool> &VisBufferStub::flag()
{
    AlwaysAssert(false, AipsError);
}

const Matrix<Bool> &VisBufferStub::flag() const
{
    AlwaysAssert(false, AipsError);
}

Cube<Bool> &VisBufferStub::flagCube()
{
    return itsFlag;
}

const Cube<Bool> &VisBufferStub::flagCube() const
{
    return itsFlag;
}

Vector<Bool> &VisBufferStub::flagRow()
{
    return itsFlagRow;
}

const Vector<Bool> &VisBufferStub::flagRow() const
{
    return itsFlagRow;
}

Array<Bool> &VisBufferStub::flagCategory()
{
    AlwaysAssert(false, AipsError);
}

const Array<Bool> &VisBufferStub::flagCategory() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Int> &VisBufferStub::scan()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Int> &VisBufferStub::scan() const
{
    AlwaysAssert(false, AipsError);
}

Int VisBufferStub::scan0()
{
    AlwaysAssert(false, AipsError);
}

Vector<Int> &VisBufferStub::processorId()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Int> &VisBufferStub::processorId() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Int> &VisBufferStub::observationId()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Int> &VisBufferStub::observationId() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Int> &VisBufferStub::stateId()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Int> &VisBufferStub::stateId() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Double> &VisBufferStub::frequency()
{
    return itsFrequency;
}

const Vector<Double> &VisBufferStub::frequency() const
{
    return itsFrequency;
}

Vector<Double> &VisBufferStub::lsrFrequency()
{
    return itsFrequency;
}

const Vector<Double> &VisBufferStub::lsrFrequency() const
{
    return itsFrequency;
}

void VisBufferStub::lsrFrequency(const Int & spw, Vector<Double>& freq,
    Bool &convert) const
{
    AlwaysAssert(false, AipsError);
}

Int VisBufferStub::numberCoh() const
{
    AlwaysAssert(false, AipsError);
}

MDirection &VisBufferStub::phaseCenter()
{
    return itsPhaseCenter;
}

const MDirection &VisBufferStub::phaseCenter() const
{
    return itsPhaseCenter;
}

Int VisBufferStub::polFrame() const
{
    return MSIter::Linear;
}

Vector<Int> &VisBufferStub::corrType()
{
    return itsCorrType;
}

const Vector<Int> &VisBufferStub::corrType() const
{
    return itsCorrType;
}

Vector<Float> &VisBufferStub::sigma()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Float> &VisBufferStub::sigma() const
{
    AlwaysAssert(false, AipsError);
}

Matrix<Float> &VisBufferStub::sigmaMat()
{
    AlwaysAssert(false, AipsError);
}

const Matrix<Float> &VisBufferStub::sigmaMat() const
{
    AlwaysAssert(false, AipsError);
}

Int &VisBufferStub::spectralWindow()
{
    return itsSpectralWindow;
}

Int VisBufferStub::spectralWindow() const
{
    return itsSpectralWindow;
}

Int VisBufferStub::dataDescriptionId() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Double> &VisBufferStub::time()
{
    return itsTime;
}

const Vector<Double> &VisBufferStub::time() const
{
    return itsTime;
}

Vector<Double> &VisBufferStub::timeCentroid()
{
    return itsTimeCentroid;
}

const Vector<Double> &VisBufferStub::timeCentroid() const
{
    return itsTimeCentroid;
}

Vector<Double> &VisBufferStub::timeInterval()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Double> &VisBufferStub::timeInterval() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Double> &VisBufferStub::exposure()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Double> &VisBufferStub::exposure() const
{
    AlwaysAssert(false, AipsError);
}

Vector<RigidVector<Double, 3> > &VisBufferStub::uvw()
{
    return itsUVW;
}

const Vector<RigidVector<Double, 3> > &VisBufferStub::uvw() const
{
    return itsUVW;
}

Matrix<Double> &VisBufferStub::uvwMat()
{
    AlwaysAssert(false, AipsError);
}

const Matrix<Double> &VisBufferStub::uvwMat() const
{
    AlwaysAssert(false, AipsError);
}

Matrix<CStokesVector> &VisBufferStub::visibility()
{
    AlwaysAssert(false, AipsError);
}

const Matrix<CStokesVector> &VisBufferStub::visibility() const
{
    AlwaysAssert(false, AipsError);
}

Matrix<CStokesVector> &VisBufferStub::modelVisibility()
{
    AlwaysAssert(false, AipsError);
}

const Matrix<CStokesVector> &VisBufferStub::modelVisibility() const
{
    AlwaysAssert(false, AipsError);
}

Matrix<CStokesVector> &VisBufferStub::correctedVisibility()
{
    AlwaysAssert(false, AipsError);
}

const Matrix<CStokesVector> &VisBufferStub::correctedVisibility() const
{
    AlwaysAssert(false, AipsError);
}

Cube<Complex> &VisBufferStub::visCube()
{
    AlwaysAssert(false, AipsError);
}

const Cube<Complex> &VisBufferStub::visCube() const
{
    return itsData;
}

Cube<Complex> &VisBufferStub::modelVisCube()
{
    return itsData;
}

Cube<Complex> &VisBufferStub::modelVisCube(const Bool & matchVisCubeShape)
{
    return itsData;
}

const Cube<Complex> &VisBufferStub::modelVisCube() const
{
    return itsData;
}

Cube<Complex> &VisBufferStub::correctedVisCube()
{
    AlwaysAssert(false, AipsError);
}

const Cube<Complex> &VisBufferStub::correctedVisCube() const
{
    AlwaysAssert(false, AipsError);
}

Cube<Float> &VisBufferStub::floatDataCube()
{
    AlwaysAssert(false, AipsError);
}

const Cube<Float> &VisBufferStub::floatDataCube() const
{
    AlwaysAssert(false, AipsError);
}

Vector<Float> &VisBufferStub::weight()
{
    AlwaysAssert(false, AipsError);
}

const Vector<Float> &VisBufferStub::weight() const
{
    AlwaysAssert(false, AipsError);
}

Matrix<Float> &VisBufferStub::weightMat()
{
    AlwaysAssert(false, AipsError);
}

const Matrix<Float> &VisBufferStub::weightMat() const
{
    AlwaysAssert(false, AipsError);
}

Bool VisBufferStub::existsWeightSpectrum() const
{
    AlwaysAssert(false, AipsError);
}

Cube<Float> &VisBufferStub::weightSpectrum()
{
    AlwaysAssert(false, AipsError);
}

const Cube<Float> &VisBufferStub::weightSpectrum() const
{
    AlwaysAssert(false, AipsError);
}

Matrix<Float> &VisBufferStub::imagingWeight()
{
    return itsImagingWeight;
}

const Matrix<Float> &VisBufferStub::imagingWeight() const
{
    return itsImagingWeight;
}

Cube<Float> &VisBufferStub::weightCube()
{
    AlwaysAssert(false, AipsError);
}

Vector<Int> VisBufferStub::vecIntRange(const MSCalEnums::colDef & calEnum) const
{
    AlwaysAssert(false, AipsError);
}

Vector<Int> VisBufferStub::antIdRange() const
{
    AlwaysAssert(false, AipsError);
}

Bool VisBufferStub::timeRange(MEpoch &rTime, MVEpoch &rTimeEP,
    MVEpoch &rInterval) const
{
    AlwaysAssert(false, AipsError);
}

Vector<uInt> &VisBufferStub::rowIds()
{
    AlwaysAssert(false, AipsError);
}

const Vector<uInt> &VisBufferStub::rowIds() const
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::freqAverage()
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::chanAveFlagCube(Cube<Bool>& flagcube, const Int nChanOut,
    const Bool restoreWeightSpectrum)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::formStokes()
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::formStokesWeightandFlag()
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::formStokes(Cube<Complex>& vis)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::formStokes(Cube<Float>& fcube)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::sortCorr()
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::unSortCorr()
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::normalize(const Bool & phaseOnly)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::resetWeightMat()
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::phaseCenterShift(Double dx, Double dy)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::updateCoordInfo(const VisBuffer *vb,
    const Bool dirDependent)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::setVisCube(Complex c)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::setModelVisCube(Complex c)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::setCorrectedVisCube(Complex c)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::setVisCube(const Cube<Complex>& vis)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::setModelVisCube(const Cube<Complex>& vis)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::setCorrectedVisCube(const Cube<Complex>& vis)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::setFloatDataCube(const Cube<Float>& fcube)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::setModelVisCube(const Vector<Float>& stokes)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::refModelVis(const Matrix<CStokesVector>& mvis)
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::removeScratchCols()
{
    AlwaysAssert(false, AipsError);
}

const ROMSColumns &VisBufferStub::msColumns() const
{
    return itsROMSColumns;
}

Int VisBufferStub::numberAnt() const
{
    AlwaysAssert(false, AipsError);
}

void VisBufferStub::allSelectedSpectralWindows(Vector<Int>& spws,
    Vector<Int>& nvischan)
{
    spws.resize(1);
    spws(0) = 0;
    nvischan.resize(1);
    nvischan(0) = itsFrequency.size();
}

void VisBufferStub::allSelectedSpectralWindows(Vector<Int>& spws,
    Vector<Int>& nvischan) const
{
    spws.resize(1);
    spws(0) = 0;
    nvischan.resize(1);
    nvischan(0) = itsFrequency.size();
}

Int VisBufferStub::msId() const
{
    return 0;
}

Bool VisBufferStub::newMS() const
{
    return itsNewMS;
}

} //# namespace casaimwrap
} //# namespace LOFAR
