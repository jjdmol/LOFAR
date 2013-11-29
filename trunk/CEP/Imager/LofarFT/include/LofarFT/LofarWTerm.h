//# LofarWTerm.h: Compute the LOFAR W-term
//#
//# Copyright (C) 2011
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

#ifndef LOFAR_LOFARFT_LOFARWTERM_H
#define LOFAR_LOFARFT_LOFARWTERM_H

#include <Common/LofarTypes.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>

namespace LOFAR {

  class WScale
  {
  public:
    WScale()
      : m_scale (0.0)
    {
    }

    WScale(double maxW, uint nPlanes)
      : m_scale (0.0)
    {
      if (nPlanes > 1) {
        m_scale = maxW / (nPlanes - 1);
      }
    }

    double lower(uint i_plane) const
    {
      if (i_plane == 0) {
        return 0.0;
      }

      return ( i_plane + 0.5) * m_scale;
    }

    double upper(uint i_plane) const
    {
      return ( i_plane + 0.5) * m_scale;
    }

    double center(uint i_plane) const
    {
      return  i_plane * m_scale;
    }

    uint plane(double w) const
    {
      if (m_scale == 0.0) {
        return 0;
      }
      w = abs(w);
      uint estimate = floor(w / m_scale);
      return w > upper(estimate) ? estimate + 1 : estimate;

    }



    /* WScale(double maxW, uint nPlanes) */
    /*   : m_scale (0.0) */
    /* { */
    /*   if (nPlanes > 1) { */
    /*     m_scale = maxW / ((nPlanes - 1) * (nPlanes - 1)); */
    /*   } */
    /* } */

    /* double lower(uint i_plane) const */
    /* { */
    /*   if (i_plane == 0) { */
    /*     return 0.0; */
    /*   } */

    /*   return (i_plane * i_plane - i_plane + 0.5) * m_scale; */
    /* } */

    /* double upper(uint i_plane) const */
    /* { */
    /*   return (i_plane * i_plane + i_plane + 0.5) * m_scale; */
    /* } */

    /* double center(uint i_plane) const */
    /* { */
    /*   return i_plane * i_plane * m_scale; */
    /* } */

    /* uint plane(double w) const */
    /* { */
    /*   if (m_scale == 0.0) { */
    /*     return 0; */
    /*   } */
    /*   w = abs(w); */
    /*   uint estimate = floor(sqrt(w / m_scale)); */
    /*   return w > upper(estimate) ? estimate + 1 : estimate; */
    /* } */


  private:
    //# Data members.
    double m_scale;
  };

  class LinearScale
  {
  public:
    LinearScale()
      : m_origin   (0.0),
        m_interval (1.0)
    {
    }

    LinearScale(double origin, double interval)
      : m_origin   (origin),
        m_interval (interval)
    {
    }

    double lower(uint bin) const
    {
      return value(bin);
    }

    double upper(uint bin) const
    {
      return value(bin + 1);
    }

    double center(uint bin) const
    {
      return value(bin + 0.5);
    }

    uint bin(double x) const
    {
      return uint((x - m_origin) / m_interval);
    }

  private:
    double value(double bin) const
    {
      return bin * m_interval + m_origin;
    }

    //# Data members.
    double  m_origin;
    double  m_interval;
  };

  // Class to compute the W-term on the image plane.
  class LofarWTerm
  {
  public:
    // The assumption here seems to be that the user images the field of view and
    // that he specifies a cellsize that correctly samples the beam. This cellsize
    // can be estimated as approximately lambda / (4.0 * B), where B is the maximal
    // baseline length in meters.

    // So it seems to be assumed that abs(resolution(0)) ~ lambda / (4 * B), therefore
    // maxW = 0.25 / (lambda / (4 * B)) = (4 * B) / (4 * lambda) = B / lambda.
    // This is exactly equal to the longest baselines in wavelengths.
    // Double maxW = 0.25 / abs(coordinates.increment()(0));
    // logIO << LogIO::NORMAL << "Estimated maximum W: " << maxW << " wavelengths." << LogIO::POST;
    // m_scale = Double(nPlanes - 1) * Double(nPlanes - 1) / maxW;
    // logIO << LogIO::NORMAL << "Scaling in W (at maximum W): " << 1.0 / m_scale
    // << " wavelengths/pixel." << LogIO::POST;

    casa::Matrix<casa::Complex> evaluate
    (const casa::IPosition &shape,
     const casa::Vector<casa::Double>& resolution,
     double w) const;

    void evaluate(casa::Complex* buffer,
                  int nx, int ny,
                  const casa::Vector<casa::Double>& resolution,
                  double w) const;

    casa::Complex evaluate_pixel(int x, int y, int nx, int ny,
				 const casa::Vector<casa::Double>& resolution,
				 double w) const;
  };

} // end namespace

#endif
