//# ExprVisDataImage.h: 
//#
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
//# $Id: ExprVisDataImage.h 20029 2012-02-20 15:50:23Z duscha $


#include <ModelImageFFT.h>

class ExprVisDataImage
{
public:
  typedef shared_ptr<ExprVisDataImage>        Ptr;
  typedef shared_ptr<const ExprVisDataImage>  ConstPtr;
    
  ExprVisDataImage( const Expr<Vector<3> >::ConstPtr &uvwA,     // node A
                    const Expr<Vector<3> >::ConstPtr &uvwB,     // node B
                    const Vector<double> &times,
                    const Vector<double> &freqs);
  ~ExprVisDataImage();

/* // GaussianCoherence class as an example
GaussianCoherence::GaussianCoherence(const Expr<Vector<4> >::ConstPtr &stokes,
    const Expr<Vector<2> >::ConstPtr &dimensions,
    const Expr<Scalar>::ConstPtr &orientation,
    const Expr<Vector<3> >::ConstPtr &uvwA,
    const Expr<Vector<3> >::ConstPtr &uvwB)
    :   BasicExpr5<Vector<4>, Vector<2>, Scalar, Vector<3>, Vector<3>,
            JonesMatrix>(stokes, dimensions, orientation, uvwA, uvwB)
{
}
*/

  // Access functions to get uvw for a particular baseline
  JonesMatrix getVisData();
  JonesMatrix getVisData( const Vector<double> &times, const Vector<double> &freqs);  // w/o A-Projection
  // w/ A-Projection we need to provide both baseline stations
  JonesMatrix getVisData( unsigned int stationA, unsigned int stationB);
  JonesMatrix getVisData( const Vector<double> &times, const Vector<double> &freqs,   // A-projection
                          unsigned int station1, unsigned int station2);

private:
  ModelImageFft::ConstPtr ModelImage;         // pointer to ModelImage class containing FFT'ed image

  itsUvwA;
  itsUvwB;

  Expr<Vector<3> > computeBaseline();         // compute baseline
};