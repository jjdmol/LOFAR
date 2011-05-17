//# LOFARConvolutionFunction.h: Compute LOFAR convolution functions on demand.
//#
//#
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef SYNTHESIS_LOFARCONVOLUTIONFUNCTION_H
#define SYNTHESIS_LOFARCONVOLUTIONFUNCTION_H

#include <casa/Logging/LogIO.h>
//#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/PagedImage.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Assert.h>
#include <synthesis/MeasurementComponents/LOFARATerm.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>

namespace casa
{

template <class T>
void store(const Matrix<T> &data, const string &name);

template <class T>
void store(const Cube<T> &data, const string &name);

class WScale
{
public:
    WScale()
        :   m_scale(1.0)
    {
    }

    WScale(Double maxW, uInt nPlanes)
        :   m_scale(1.0)
    {
        if(nPlanes > 1)
        {
            m_scale = (nPlanes * nPlanes) / maxW;
        }
    }

    Double lower(uInt plane) const
    {
        if(plane == 0)
        {
            return 0.0;
        }

        return value(static_cast<Double>(plane) - 0.5);
    }

    Double upper(uInt plane) const
    {
        return value(static_cast<Double>(plane) + 0.5);
    }

    Double center(uInt plane) const
    {
        return value(static_cast<Double>(plane));
    }

    uInt plane(Double w) const
    {
        return floor(sqrt(abs(w) * m_scale) + 0.5);
    }

private:
    Double value(Double x) const
    {
        return (x * x) / m_scale;
    }

    Double  m_scale;
};

class LinearScale
{
public:
    LinearScale()
        :   m_origin(0.0),
            m_interval(1.0)
    {
    }

    LinearScale(Double origin, Double interval)
        :   m_origin(origin),
            m_interval(interval)
    {
    }

    Double lower(uInt bin) const
    {
        return value(bin);
    }

    Double upper(uInt bin) const
    {
        return value(bin + 1);
    }

    Double center(uInt bin) const
    {
        return value(static_cast<Double>(bin) + 0.5);
    }

    Int bin(Double x) const
    {
        return uInt((x - m_origin) / m_interval);
    }

private:
    Double value(Double bin) const
    {
        return bin * m_interval + m_origin;
    }

    Double  m_origin;
    Double  m_interval;
};

// Class to compute the W-term on the image plane.
class LOFARWTerm
{
public:
//    LOFARWTerm(uInt nPlanes, const DirectionCoordinate &coordinates)
//        :   m_nPlanes(nPlanes),
//            m_scale(1.0)
//    {
//        LogIO logIO(LogOrigin("LOFARWTerm", "LOFARWTerm"));
//        logIO << LogIO::NORMAL << "Using " << nPlanes << " planes for W-projection." << LogIO::POST;

//        if(m_nPlanes > 1)
//        {
//            // The assumption here seems to be that the user images the field of view and
//            // that he specifies a cellsize that correctly samples the beam. This cellsize
//            // can be estimated as approximately lambda / (4.0 * B), where B is the maximal
//            // baseline length in meters.

//            // So it seems to be assumed that abs(resolution(0)) ~ lambda / (4 * B), therefore
//            // maxW = 0.25 / (lambda / (4 * B)) = (4 * B) / (4 * lambda) = B / lambda.
//            // This is exactly equal to the longest baselines in wavelengths.
//            Double maxW = 0.25 / abs(coordinates.increment()(0));
//            logIO << LogIO::NORMAL << "Estimated maximum W: " << maxW << " wavelengths." << LogIO::POST;

//            m_scale = Double(nPlanes - 1) * Double(nPlanes - 1) / maxW;
//            logIO << LogIO::NORMAL << "Scaling in W (at maximum W): " << 1.0 / m_scale
//                << " wavelengths/pixel." << LogIO::POST;
//        }
//    }

    Matrix<Complex> evaluate(const IPosition &shape, const DirectionCoordinate &coordinates, Double w) const
    {
        if(w == 0)
        {
            return Matrix<Complex>(shape, 1.0);
        }

        Vector<Double> resolution = coordinates.increment();
        Double radius[2] = {shape(0) / 2.0, shape(1) / 2.0};
        Double twoPiW = 2.0 * C::pi * w;

        Matrix<Complex> plane(shape, 0.0);
        for(uInt y = 0; y < shape(1); ++y)
        {
            Double m = resolution(1) * (y - radius[1]);
            Double m2 = m * m;

            for(uInt x = 0; x < shape(0); ++x)
            {
                Double l = resolution(0) * (x - radius[0]);
                Double lm2 = l * l + m2;

                if(lm2 < 1.0)
                {
                    Double phase = twoPiW * (sqrt(1.0 - lm2) - 1.0);
                    plane(x, y) = Complex(cos(phase), sin(phase));
                }
            }
        }

        return plane;
    }

//private:
//    uInt    m_nPlanes;
//    Double  m_scale;
};

class LOFARConvolutionFunction
{
public:
    LOFARConvolutionFunction(const IPosition &shape, const DirectionCoordinate &coordinates,
        const MeasurementSet &ms, uInt nWPlanes, uInt intervalATerm)
        :   m_shape(shape),
            m_coordinates(coordinates),
            m_aTerm(ms)
    {
        // The assumption here seems to be that the user images the field of view and
        // that he specifies a cellsize that correctly samples the beam. This cellsize
        // can be estimated as approximately lambda / (4.0 * B), where B is the maximal
        // baseline length in meters.

        // So it seems to be assumed that abs(resolution(0)) ~ lambda / (4 * B), therefore
        // maxW = 0.25 / (lambda / (4 * B)) = (4 * B) / (4 * lambda) = B / lambda.
        // This is exactly equal to the longest baselines in wavelengths.
        Double maxW = 0.25 / abs(coordinates.increment()(0));
        logIO() << LogOrigin("LOFARConvolutionFunction", "LOFARConvolutionFunction") << LogIO::NORMAL
            << "Estimated maximum W: " << maxW << " wavelengths." << LogIO::POST;

        m_wScale = WScale(maxW, nWPlanes);

        MEpoch start = observationStartTime(ms, 0);
        m_timeScale = LinearScale(start.getValue().getTime("s").getValue(),
            static_cast<Double>(intervalATerm));
    }

    Cube<Complex> makeConvolutionFunction(uInt stationA, uInt stationB, const MEpoch &epoch,
        Double w) const
    {
        uInt wPlane = m_wScale.plane(w);

        Double time = epoch.getValue().getTime("s").getValue();
        uInt timeBin = m_timeScale.bin(time);
        logIO() << LogOrigin("LOFARConvolutionFunction", "makeConvolutionFunction") << LogIO::NORMAL
            << "w-plane: " << wPlane << " time bin: " << timeBin << LogIO::POST;

        // To precompute A-terms and W-term.

        // 1. Request required angular scales for A-term, W-term.
        Double wTermResolution = estimateWResolution(m_shape, m_coordinates);
        Double aTermResolution = estimateAResolution(m_shape, m_coordinates);

        // 2. Compute minimum angular scale.
        Double minResolution = min(wTermResolution, aTermResolution);
        logIO() << LogOrigin("LOFARConvolutionFunction", "makeConvolutionFunction") << LogIO::NORMAL
            << "Estimated minimum required angular size: " << minResolution << " rad/pixel" << LogIO::POST;

        // 3. Divide angular scale of image by minimum angular scale => #pixels needed for
        //    image plane functions.
        uInt nPixels = (abs(m_coordinates.increment()(0)) * m_shape(0)) / minResolution;
        logIO() << LogOrigin("LOFARConvolutionFunction", "makeConvolutionFunction") << LogIO::NORMAL
            << "Convolution function support in the image domain: " << nPixels << " pixels" << LogIO::POST;

        // 4. Create request with minimum angular scale and #pixels. Could slightly increase #pixels
        //    here to allow for a transition region for tapering.
        DirectionCoordinate coordinates(m_coordinates);
        Vector<Double> increment(minResolution, minResolution);
        coordinates.setIncrement(increment);

        IPosition shape(2, nPixels, nPixels);

        // 5. Evaluate A-terms and W-term for request.
        Matrix<Complex> wTerm = m_wTerm.evaluate(shape, coordinates, m_wScale.center(wPlane));

        MEpoch binEpoch(epoch);
        binEpoch.set(Quantity(m_timeScale.lower(timeBin), "s"));
        Cube<Complex> aTerm = m_aTerm.evaluate(shape, coordinates, 0, binEpoch);

        for(uInt i = 0; i < 4; ++i)
        {
            Matrix<Complex> plane = aTerm.xyPlane(i);
            plane *= wTerm;
        }

        // 6. Taper the W-term, pushing it to zero at the edge.
        for(uInt i = 0; i < 4; ++i)
        {
            Matrix<Complex> plane = aTerm.xyPlane(i);
            taper(plane);
        }

        // 7. Store A-terms and W-term.




        // To compute baseline convolution functions:

        // 1. Load W-term and A-terms.

        // 2. Create all Mueller elements, each multiplied by W-term.

        // 3. Zero-pad each Mueller * W term.

        // 4. FT
        ArrayLattice<Complex> lattice(aTerm);
        LatticeFFT::cfft2d(lattice);

//        Matrix<Complex> spheroidal(shape, 1.0);
//        taper(spheroidal);
//        ArrayLattice<Complex> lattice0(spheroidal);
//        LatticeFFT::cfft2d(lattice0);

//        uInt supportSpheroidal = findSupport(spheroidal, 0.001);
//        logIO() << LogOrigin("LOFARConvolutionFunction", "makeConvolutionFunction") << LogIO::NORMAL
//            << "Support of the spheroidal taper in the Fourier domain: " << supportSpheroidal
//            << " pixels" << LogIO::POST;

//        store(spheroidal, "taper-test-256.img");

//        taper(wTerm);
//        ArrayLattice<Complex> lattice1(wTerm);
//        LatticeFFT::cfft2d(lattice1);

//        uInt supportWTerm = findSupport(wTerm, 0.001);
//        logIO() << LogOrigin("LOFARConvolutionFunction", "makeConvolutionFunction") << LogIO::NORMAL
//            << "Support of the W-term in the Fourier domain: " << supportWTerm
//            << " pixels" << LogIO::POST;

        // 5. Cut out using some threshold.
        store(aTerm, "awterm-test-256.img");
        return aTerm;
    }

    const WScale &wScale() const
    {
        return m_wScale;
    }

    const LinearScale &timeScale() const
    {
        return m_timeScale;
    }

private:
    LogIO &logIO() const
    {
        return m_logIO;
    }

    MEpoch observationStartTime(const MeasurementSet &ms, uInt idObservation) const
    {
        // Get phase center as RA and DEC (J2000).
        ROMSObservationColumns observation(ms.observation());
        AlwaysAssert(observation.nrow() > idObservation, SynthesisError);
        AlwaysAssert(!observation.flagRow()(idObservation), SynthesisError);

        return observation.timeRangeMeas()(0)(IPosition(1, 0));
    }

    // Return the angular resolution required for making the image of the angular size determined by
    // coordinates and shape. The resolution is assumed to be the same on both direction axes.
    Double estimateWResolution(const IPosition &shape, const DirectionCoordinate &coordinates) const
    {
        logIO() << LogOrigin("LOFARConvolutionFunction", "estimateWResolution") << LogIO::WARN
            << "TODO: Find the proper way to estimate the angular resolution required for the W-term."
            << LogIO::POST;
        return abs(coordinates.increment()(0));
    }

    // Return the angular resolution required for making the image of the angular size determined by
    // coordinates and shape. The resolution is assumed to be the same on both direction axes.
    Double estimateAResolution(const IPosition &shape, const DirectionCoordinate &coordinates) const
    {
        logIO() << LogOrigin("LOFARConvolutionFunction", "estimateAResolution") << LogIO::WARN
            << "TODO: Find the proper way to estimate the angular resolution required for the A-term."
            << LogIO::POST;

        return 1.0 / (0.25 / abs(coordinates.increment()(0)));
    }

    // Apply a spheroidal taper to the input function.
    template <typename T>
    void taper(Matrix<T> &function) const
    {
        AlwaysAssert(function.shape()(0) == function.shape()(1), SynthesisError);

        uInt size = function.shape()(0);
        Double halfSize = size / 2.0;

        Vector<Double> x(size);
        for(uInt i = 0; i < size; ++i)
        {
            x(i) = spheroidal(abs(Double(i) - halfSize) / halfSize);
        }

        for(uInt i = 0; i < size; ++i)
        {
            for(uInt j = 0; j < size; ++j)
            {
                function(j, i) *= x(i) * x(j);
            }
        }
    }

    Double spheroidal(Double nu) const
    {
        static Double P[2][5] = {{8.203343e-2, -3.644705e-1, 6.278660e-1, -5.335581e-1, 2.312756e-1},
            {4.028559e-3, -3.697768e-2, 1.021332e-1,-1.201436e-1, 6.412774e-2}};
        static Double Q[2][3] = {{1.0000000e0, 8.212018e-1, 2.078043e-1},
            {1.0000000e0, 9.599102e-1, 2.918724e-1}};

        uInt part = 0;
        Double end = 0.0;

        if(nu >= 0.0 && nu < 0.75)
        {
            part = 0;
            end = 0.75;
        }
        else if(nu >= 0.75 && nu <= 1.00)
        {
            part = 1;
            end = 1.00;
        }
        else
        {
            return 0.0;
        }

        Double nusq = nu * nu;
        Double delnusq = nusq - end * end;

        Double top = P[part][0];
        for(uInt k = 1; k < 5; ++k)
        {
            top += P[part][k] * pow(delnusq, k);
        }

        Double bot = Q[part][0];
        for(uInt k = 1; k < 3; ++k)
        {
            bot += Q[part][k] * pow(delnusq, k);
        }

        return bot == 0.0 ? 0.0 : (1.0 - nusq) * (top / bot);
    }

    template <typename T>
    uInt findSupport(Matrix<T> &function, Double threshold) const
    {
        Double peak = abs(max(abs(function)));
        threshold *= peak;

        uInt halfSize = function.shape()(0) / 2;

        uInt x = 0;
        while(x < halfSize && abs(function(x, halfSize)) < threshold)
        {
            ++x;
        }

        return 2 * (halfSize - x);
    }

    IPosition           m_shape;
    DirectionCoordinate m_coordinates;
    WScale              m_wScale;
    LinearScale         m_timeScale;
    LOFARWTerm          m_wTerm;
    LOFARATerm          m_aTerm;
    mutable LogIO       m_logIO;
};

// Utility function to store a Matrix as an image for debugging. It uses arbitrary values for the
// direction, Stokes and frequency axes.
template <class T>
void store(const Matrix<T> &data, const string &name)
{
    CoordinateSystem csys;

    Matrix<Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
    Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
    Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
    csys.addCoordinate(DirectionCoordinate(MDirection::J2000, Projection(Projection::SIN),
        refLatLon, refLatLon, incLon, incLat,
        xform, data.shape()(0) / 2, data.shape()(1) / 2));

    Vector<Int> stokes(1);
    stokes(0) = Stokes::I;
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));

    PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
    im.putSlice(data, IPosition(4, 0, 0, 0, 0));
}

// Utility function to store a Cube as an image for debugging. It uses arbitrary values for the
// direction, Stokes and frequency axes. The size of the third axis is assumed to be 4.
template <class T>
void store(const Cube<T> &data, const string &name)
{
    AlwaysAssert(data.shape()(2) == 4, SynthesisError);

    CoordinateSystem csys;

    Matrix<Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
    Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
    Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
    csys.addCoordinate(DirectionCoordinate(MDirection::J2000, Projection(Projection::SIN),
        refLatLon, refLatLon, incLon, incLat,
        xform, data.shape()(0) / 2, data.shape()(1) / 2));

    Vector<Int> stokes(4);
    stokes(0) = Stokes::XX;
    stokes(1) = Stokes::XY;
    stokes(2) = Stokes::YX;
    stokes(3) = Stokes::YY;
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));

    PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 4, 1)), csys, name);
    im.putSlice(data, IPosition(4, 0, 0, 0, 0));
}

} // namespace casa

#endif
