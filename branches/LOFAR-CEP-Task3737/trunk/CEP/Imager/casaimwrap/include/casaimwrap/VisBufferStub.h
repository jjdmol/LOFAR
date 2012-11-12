//# VisBufferStub.h: Hacked VisBuffer implementation that can be used to pass
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

#ifndef LOFAR_CASAIMWRAP_VISBUFFERSTUB_H
#define LOFAR_CASAIMWRAP_VISBUFFERSTUB_H

// \file
// Hacked VisBuffer implementation that can be used to pass visibility data
// between Python and CASA FTMachines.

#include <msvis/MSVis/VisBuffer.h>
#include <ms/MeasurementSets/MSColumns.h>

namespace LOFAR
{
namespace casaimwrap
{
using namespace casa;

// \addtogroup casaimwrap
// @{

class VisBufferStub: public VisBuffer
{
public:
    VisBufferStub(const MeasurementSet &ms);

    void setChunk(const Vector<Int> &antenna1,
        const Vector<Int> &antenna2,
        const Matrix<Double> &uvw,
        const Vector<Double> &time,
        const Vector<Double> &timeCentroid,
        const Vector<Bool> &flagRow,
        const Matrix<Float> &weight,
        const Cube<Bool> &flag,
        const Cube<Complex> &data,
        Bool newMS = false);

    void setChunk(const Vector<Int> &antenna1,
        const Vector<Int> &antenna2,
        const Matrix<Double> &uvw,
        const Vector<Double> &time,
        const Vector<Double> &timeCentroid,
        const Vector<Bool> &flagRow,
        const Matrix<Float> &weight,
        const Cube<Bool> &flag,
        Bool newMS = false);

    virtual VisBuffer & assign(const VisBuffer & vb, Bool copy = True);
    virtual void attachToVisIter(ROVisibilityIterator & iter);
    virtual void detachFromVisIter();
    virtual void invalidate();
    virtual Int & nCorr();
    virtual Int nCorr() const;
    virtual Int & nChannel();
    virtual Int nChannel() const;
    virtual Vector<Int>& channel();
    virtual const Vector<Int>& channel() const;
    virtual Int & nRow();
    virtual Int nRow() const;
    virtual Vector<Int>& antenna1();
    virtual const Vector<Int>& antenna1() const;
    virtual Vector<Int>& antenna2();
    virtual const Vector<Int>& antenna2() const;
    virtual Vector<Int>& feed1();
    virtual const Vector<Int>& feed1() const;
    virtual Vector<Int>& feed2();
    virtual const Vector<Int>& feed2() const;
    virtual Vector<Float>& feed1_pa();
    virtual const Vector<Float>& feed1_pa() const;
    virtual Vector<Float>& feed2_pa();
    virtual const Vector<Float>& feed2_pa() const;
    virtual Vector<SquareMatrix<Complex, 2> >& CJones();
    virtual const Vector<SquareMatrix<Complex, 2> >& CJones() const;
    virtual Vector<Float> feed_pa(Double time) const;
    virtual Vector<MDirection>& direction1();
    virtual const Vector<MDirection>& direction1() const;
    virtual Vector<MDirection>& direction2();
    virtual const Vector<MDirection>& direction2() const;
    virtual Float parang0(Double time) const;
    virtual Vector<Float> parang(Double time) const;
    virtual MDirection azel0(Double time) const;
    virtual Vector<Double>& azel0Vec(Double time, Vector<Double>& azelVec)
        const;
    virtual Vector<MDirection> azel(Double time) const;
    virtual Matrix<Double>& azelMat(Double time, Matrix<Double>& azelMat) const;
    virtual Double hourang(Double time) const;
    virtual Int fieldId() const;
    virtual Int arrayId() const;
    virtual Matrix<Bool>& flag();
    virtual const Matrix<Bool>& flag() const;
    virtual Cube<Bool>& flagCube();
    virtual const Cube<Bool>& flagCube() const;
    virtual Vector<Bool>& flagRow();
    virtual const Vector<Bool>& flagRow() const;
    virtual Array<Bool>& flagCategory();
    virtual const Array<Bool>& flagCategory() const;
    virtual Vector<Int>& scan();
    virtual const Vector<Int>& scan() const;
    virtual Int scan0();
    virtual Vector<Int>& processorId();
    virtual const Vector<Int>& processorId() const;
    virtual Vector<Int>& observationId();
    virtual const Vector<Int>& observationId() const;
    virtual Vector<Int>& stateId();
    virtual const Vector<Int>& stateId() const;
    virtual Vector<Double>& frequency();
    virtual const Vector<Double>& frequency() const;
    virtual Vector<Double>& lsrFrequency();
    virtual const Vector<Double>& lsrFrequency() const;
    virtual void lsrFrequency(const Int & spw, Vector<Double>& freq,
        Bool & convert) const;
    virtual Int numberCoh () const;
    virtual MDirection & phaseCenter();
    virtual const MDirection & phaseCenter() const;
    virtual Int polFrame() const;
    virtual Vector<Int>& corrType();
    virtual const Vector<Int>& corrType() const;
    virtual Vector<Float>& sigma();
    virtual const Vector<Float>& sigma() const;
    virtual Matrix<Float>& sigmaMat();
    virtual const Matrix<Float>& sigmaMat() const;
    virtual Int & spectralWindow();
    virtual Int spectralWindow() const;
    virtual Int dataDescriptionId() const;
    virtual Vector<Double>& time();
    virtual const Vector<Double>& time() const;
    virtual Vector<Double>& timeCentroid();
    virtual const Vector<Double>& timeCentroid() const;
    virtual Vector<Double>& timeInterval();
    virtual const Vector<Double>& timeInterval() const;
    virtual Vector<Double>& exposure();
    virtual const Vector<Double>& exposure() const;
    virtual Vector<RigidVector<Double, 3> >& uvw();
    virtual const Vector<RigidVector<Double, 3> >& uvw() const;
    virtual Matrix<Double>& uvwMat();
    virtual const Matrix<Double>& uvwMat() const;
    virtual Matrix<CStokesVector>& visibility();
    virtual const Matrix<CStokesVector>& visibility() const;
    virtual Matrix<CStokesVector>& modelVisibility();
    virtual const Matrix<CStokesVector>& modelVisibility() const;
    virtual Matrix<CStokesVector>& correctedVisibility();
    virtual const Matrix<CStokesVector>& correctedVisibility() const;
    virtual Cube<Complex>& visCube();
    virtual const Cube<Complex>& visCube() const;
    virtual Cube<Complex>& modelVisCube();
    virtual Cube<Complex>& modelVisCube(const Bool & matchVisCubeShape);
    virtual const Cube<Complex>& modelVisCube() const;
    virtual Cube<Complex>& correctedVisCube();
    virtual const Cube<Complex>& correctedVisCube() const;
    virtual Cube<Float>& floatDataCube();
    virtual const Cube<Float>& floatDataCube() const;
    virtual Vector<Float>& weight();
    virtual const Vector<Float>& weight() const;
    virtual Matrix<Float>& weightMat();
    virtual const Matrix<Float>& weightMat() const;
    virtual Bool existsWeightSpectrum() const;
    virtual Cube<Float>& weightSpectrum();
    virtual const Cube<Float>& weightSpectrum() const;
    virtual Matrix<Float>& imagingWeight();
    virtual const Matrix<Float>& imagingWeight() const;
    virtual Cube<Float>& weightCube();
    virtual Vector<Int> vecIntRange(const MSCalEnums::colDef & calEnum) const;
    virtual Vector<Int> antIdRange() const;
    virtual Bool timeRange(MEpoch & rTime, MVEpoch & rTimeEP,
        MVEpoch & rInterval) const;
    virtual Vector<uInt>& rowIds();
    virtual const Vector<uInt>& rowIds() const;
    virtual void freqAverage();
    virtual void chanAveFlagCube(Cube<Bool>& flagcube, const Int nChanOut,
                         const Bool restoreWeightSpectrum = True);
    virtual void formStokes();
    virtual void formStokesWeightandFlag();
    virtual void formStokes(Cube<Complex>& vis);
    virtual void formStokes(Cube<Float>& fcube);
    virtual void sortCorr();
    virtual void unSortCorr();
    virtual void normalize(const Bool & phaseOnly = False);
    virtual void resetWeightMat();
    virtual void phaseCenterShift(Double dx, Double dy);
    virtual void updateCoordInfo(const VisBuffer * vb = NULL,
        const Bool dirDependent=True);
    virtual void setVisCube(Complex c);
    virtual void setModelVisCube(Complex c);
    virtual void setCorrectedVisCube(Complex c);
    virtual void setVisCube(const Cube<Complex>& vis);
    virtual void setModelVisCube(const Cube<Complex>& vis);
    virtual void setCorrectedVisCube(const Cube<Complex>& vis);
    virtual void setFloatDataCube(const Cube<Float>& fcube);
    virtual void setModelVisCube(const Vector<Float>& stokes);
    virtual void refModelVis(const Matrix<CStokesVector>& mvis);
    virtual void removeScratchCols();
    virtual const ROMSColumns & msColumns() const;
    virtual Int numberAnt () const;
    virtual void allSelectedSpectralWindows(Vector<Int>& spws,
        Vector<Int>& nvischan);
    virtual void allSelectedSpectralWindows(Vector<Int>& spws,
        Vector<Int>& nvischan) const;
    virtual Int msId() const;
    virtual Bool newMS() const;

private:
    ROMSColumns                         itsROMSColumns;
    Vector<Double>                      itsFrequency;
    MDirection                          itsPhaseCenter;
    Vector<Int>                         itsCorrType;
    Matrix<Float>                       itsImagingWeight;
    Vector<Int>                         itsAntenna1;
    Vector<Int>                         itsAntenna2;
    Vector<RigidVector<Double, 3> >     itsUVW;
    Vector<Bool>                        itsFlagRow;
    Vector<Double>                      itsTimeCentroid;
    Vector<Double>                      itsTime;
    Cube<Bool>                          itsFlag;
    Matrix<Float>                       itsWeight;
    Cube<Complex>                       itsData;
    Bool                                itsNewMS;
    Int                                 itsSpectralWindow;
    Int                                 itsNRow;
    Int                                 itsNChannel;
    Int                                 itsNCorr;
};

// @}

} //# namespace casaimwrap
} //# namespace LOFAR

#endif
