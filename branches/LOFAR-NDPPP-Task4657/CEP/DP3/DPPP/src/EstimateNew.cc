//# EstimateNew.cc: Estimate Jones matrices for several directions and stations
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
#include <DPPP/EstimateMixed.h>
#include <Common/LofarLogger.h>
#include <scimath/Fitting/LSQFit.h>

namespace LOFAR {
  namespace DPPP {

    EstimateNew::EstimateNew()
    {}

    EstimateNew::update (size_t ndir, size_t nBaseline, size_t nStation,
                         size_t maxIter, bool propagateSolution)
    {
      itsUnknowns.resize (ndir * nStation * 4 * 2);
      itsSolution.resize (itsUnknowns.size());
      initSolution (&(itsSolution[0]), itsSolution.size());
      ///itsDerivIndex.resize (nBaseline * ndir * 32);
      itsDerivIndex.resize (ndir*32);
      itsM.resize  (ndir*4);
      itsdM.resize (ndir*16);
      itsdR.resize (ndir*8);
      itsdI.resize (ndir*8);
      itsMaxIter = maxIter;
      itsPropagateSolution = propagateSolution;
    }

    void EstimateNew::initSolution (double* solution, size_t nr)
    {
      // 4 complex unknowns, thus 8 real unknowns.
      // Initialize to 1+0i on the diagonal, elsewhere 0+0i.
      for (size_t i=0; i<nr; i+=8) {
        solution[i+0] = 1.;
        solution[i+1] = 0.;
        solution[i+2] = 0.;
        solution[i+3] = 0.;
        solution[i+4] = 0.;
        solution[i+5] = 0.;
        solution[i+6] = 1.;
        solution[i+7] = 0.;
      }
    }

    void EstimateNew::fillUnknowns (const vector<Vector<Int> >& unknownsIndex)
    {
      // Fill the unknowns for the direction-stations to solve.
      // Copy previous solution if needed.
      double* unknowns = &(itsUnknowns[0]);
      const double* solution = &(itsSolution[0]);
      for (size_t dr=0; dr<unknownsIndex.size(); ++dr) {
        for (size_t st=0; st<unknownsIndex.size(); ++st) {
          if (unknownsIndex[dr][st] >= 0) {
            if (itsPropagateSolution) {
              for (size_t k=0; k<8; ++k) {
                *unknowns++ = *solution++;
              }
            } else {
              initSolution (unknowns, 8);
              unknowns += 8;
            }
          } else {
            // Skip this solution.
            solution += 8;
          }
        }
      }
    }

    void EstimateNew::fillSolution (const vector<Vector<Int> >& unknownsIndex)
    {
      // Copy the solution for the direction-stations to solve.
      // Use initial values for other direction-stations.
      const double* unknowns = &(itsUnknowns[0]);
      double* solution = &(itsSolution[0]);
      for (size_t dr=0; dr<unknownsIndex.size(); ++dr) {
        for (size_t st=0; st<unknownsIndex.size(); ++st) {
          if (unknownsIndex[dr][st] >= 0) {
            for (size_t k=0; k<8; ++k) {
              *solution++ = *unknowns++;
            }
          } else {
            initSolution (solution, 8);
            solution += 8;
          }
        }
      }
    }

    // Form the partial derivative index for a particular baseline.
    // Partial derivatives are only needed if direction-station is solved for.
    // Each visibility provides information about two (complex) unknowns per
    // station per direction. A visibility is measured by a specific
    // interferometer, which is the combination of two stations. Thus, in total
    // each visibility provides information about (no. of directions) x 2 x 2
    // x 2 (scalar) unknowns = (no. of directions) x 8. For each of these
    // unknowns the value of the partial derivative of the model with respect
    // to the unknown has to be computed.
    uint EstimateNew::makeDerivIndex (const vector<Vector<Int> >& unknownsIndex,
                                      const Baseline& baseline)
    {
      // Per direction a baseline has information about 16 unknowns:
      // real and imag part of p00,p01,p10,p11,q00,q01,q10,q11
      // where p and q are the stations forming the baseline.
      // However, only fill if a station has to be solved.
      size_t n = 0;
      for (size_t cr=0; cr<4; ++cr) {
        for (size_t dr=0; dr<unknownsIndex.size(); ++dr) {
          size_t idx0 = 8*unknownsIndex[dr][baseline.first];
          if (idx0 >= 0) {
            for (size_t k=0; k<8; ++k) {
              itsIndex[n++] = idx0+k;
            }
          }
          size_t idx1 = 8*unknownsIndex[dr][baseline.second];
          if (idx1 >= 0) {
            for (size_t k=0; k<8; ++k) {
              itsIndex[n++] = idx1+k;
            }
          }
        }
      }
      return n;
    }

    // Note that the cursors are passed by value, so a copy is made.
    // In this way no explicit reset of the cursor is needed on a next call.
    bool EstimateNew::estimate (const vector<Vector<Int> >& unknownsIndex,
                                const Block<bool>& solveStations,
                                const_cursor<Baseline> baselines,
                                vector<const_cursor<fcomplex> > data,
                                vector<const_cursor<dcomplex> > model,
                                const_cursor<bool> flag,
                                const_cursor<float> weight,
                                const_cursor<dcomplex> mix)
    {
      // Initialize LSQ solver.
      casa::LSQFit solver(nUnknowns);
      const size_t nDirection = dirStations.size();

      // Iterate until convergence.
      size_t nIterations = 0;
      while (!solver.isReady()  &&  nIterations < maxIter) {
        for (size_t bl=0; bl<nBaseline; ++bl) {
          const size_t p = baselines->first;
          const size_t q = baselines->second;
          // Only compute if no autocorr and if a station needs to be solved.
          if (p != q  &&  (solveStation[p] || solveStation[q])) {
            // Create partial derivative index for current baseline.
            size_t nPartial = makeDerivIndex (unknownsIndex, *baselines);

            for (size_t ch=0; ch<nChannel; ++ch) {
              for (size_t dr=0; dr<nDirection; ++dr) {
                // Jones matrix for station P.
                const double *Jp = &(itsSolution[dr * nStation * 8 + p * 8]);
                const dcomplex Jp_00(Jp[0], Jp[1]);
                const dcomplex Jp_01(Jp[2], Jp[3]);
                const dcomplex Jp_10(Jp[4], Jp[5]);
                const dcomplex Jp_11(Jp[6], Jp[7]);

                // Jones matrix for station Q, conjugated.
                const double *Jq = &(itsSolution[dr * nStation * 8 + q * 8]);
                const dcomplex Jq_00(Jq[0], -Jq[1]);
                const dcomplex Jq_01(Jq[2], -Jq[3]);
                const dcomplex Jq_10(Jq[4], -Jq[5]);
                const dcomplex Jq_11(Jq[6], -Jq[7]);

                // Fetch model visibilities for the current direction.
                const dcomplex xx = model[dr][0];
                const dcomplex xy = model[dr][1];
                const dcomplex yx = model[dr][2];
                const dcomplex yy = model[dr][3];

                // Precompute terms involving conj(Jq) and the model
                // visibilities.
                const dcomplex Jq_00xx_01xy = Jq_00 * xx + Jq_01 * xy;
                const dcomplex Jq_00yx_01yy = Jq_00 * yx + Jq_01 * yy;
                const dcomplex Jq_10xx_11xy = Jq_10 * xx + Jq_11 * xy;
                const dcomplex Jq_10yx_11yy = Jq_10 * yx + Jq_11 * yy;

                // Precompute (Jp x conj(Jq)) * vec(data), where 'x'
                // denotes the Kronecker product. This is the model
                // visibility for the current direction, with the
                // current Jones matrix estimates applied. This is
                // stored in M.
                // Also, precompute the partial derivatives of M with
                // respect to all 16 parameters (i.e. 2 Jones matrices
                // Jp and Jq, 4 complex scalars per Jones matrix, 2 real
                // scalars per complex scalar, 2 * 4 * 2 = 16). These
                // partial derivatives are stored in dM.
                // Note that conj(Jq) is used and that q01 and q10 are swapped.

                itsM[dr * 4] = Jp_00 * Jq_00xx_01xy + Jp_01 * Jq_00yx_01yy;
                // Derivatives of M00 wrt p00, p01, q00, q01
                itsdM[dr * 16] = Jq_00xx_01xy;
                itsdM[dr * 16 + 1] = Jq_00yx_01yy;
                itsdM[dr * 16 + 2] = Jp_00 * xx + Jp_01 * yx;
                itsdM[dr * 16 + 3] = Jp_00 * xy + Jp_01 * yy;

                itsM[dr * 4 + 1] = Jp_00 * Jq_10xx_11xy + Jp_01 * Jq_10yx_11yy;
                // Derivatives of M01 wrt p00, p01, q10, q11
                itsdM[dr * 16 + 4] = Jq_10xx_11xy;
                itsdM[dr * 16 + 5] = Jq_10yx_11yy;
                itsdM[dr * 16 + 6] = itsdM[dr * 16 + 2];
                itsdM[dr * 16 + 7] = itsdM[dr * 16 + 3];

                itsM[dr * 4 + 2] = Jp_10 * Jq_00xx_01xy + Jp_11 * Jq_00yx_01yy;
                // Derivatives of M10 wrt p10, p11, q00, q01
                itsdM[dr * 16 + 8] = itsdM[dr * 16];
                itsdM[dr * 16 + 9] = itsdM[dr * 16 + 1];
                itsdM[dr * 16 + 10] = Jp_10 * xx + Jp_11 * yx;
                itsdM[dr * 16 + 11] = Jp_10 * xy + Jp_11 * yy;

                itsM[dr * 4 + 3] = Jp_10 * Jq_10xx_11xy + Jp_11 * Jq_10yx_11yy;
                // Derivatives of M11 wrt p10, p11, q10, q11
                itsdM[dr * 16 + 12] = itsdM[dr * 16 + 4];
                itsdM[dr * 16 + 13] = itsdM[dr * 16 + 5];
                itsdM[dr * 16 + 14] = itsdM[dr * 16 + 10];
                itsdM[dr * 16 + 15] = itsdM[dr * 16 + 11];
              }

              // Now compute the equations (per pol) for D*M=A where
              //  D is the NxN demixing weight matrix
              //  M is the model visibilities vector for the N directions
              //  A is the shifted observed visibilities vector for N directions
              // Note that each element in the vectors is a 2x2 matrix
              // (xx,xy,yx,yy) of complex values.
              // A complex multiplication of (a,b) and (c,d) gives (ac-bd,ad+bc)
              // Thus real partial derivatives wrt a,b,c,d are c,-d,a,-b.
              // Imaginary partial derivatives wrt a,b,c,d are d,c,b,a
              for (size_t cr=0; cr<4; ++cr) {
                if (!flag[cr]) {
                  for (size_t tg=0; tg<nDirection; ++tg) {
                    dcomplex visibility(0.0, 0.0);
                    for (size_t dr=0; dr<nDirection; ++dr) {
                      // Look-up mixing weight.
                      const dcomplex mix_weight = *mix;
                      // Sum weighted model visibility.
                      visibility += mix_weight * M[dr * 4 + cr];

                      // Compute weighted partial derivatives.
                      dcomplex derivative(0.0, 0.0);
                      derivative = mix_weight * itsdM[dr * 16 + cr * 4];
                      itsdR[dr * 8] = real(derivative);
                      itsdI[dr * 8] = imag(derivative);
                      itsdR[dr * 8 + 1] = -imag(derivative);
                      itsdI[dr * 8 + 1] = real(derivative);

                      derivative = mix_weight * itsdM[dr * 16 + cr * 4 + 1];
                      itsdR[dr * 8 + 4] = real(derivative);
                      itsdI[dr * 8 + 4] = imag(derivative);
                      itsdR[dr * 8 + 5] = -imag(derivative);
                      itsdI[dr * 8 + 5] = real(derivative);

                      derivative = mix_weight * itsdM[dr * 16 + cr * 4 + 2];
                      itsdR[dr * 8 + 2] = real(derivative);
                      itsdI[dr * 8 + 2] = imag(derivative);
                      itsdR[dr * 8 + 3] = imag(derivative);
                      itsdI[dr * 8 + 3] = -real(derivative);

                      derivative = mix_weight * itsdM[dr * 16 + cr * 4 + 3];
                      itsdR[dr * 8 + 6] = real(derivative);
                      itsdI[dr * 8 + 6] = imag(derivative);
                      itsdR[dr * 8 + 7] = imag(derivative);
                      itsdI[dr * 8 + 7] = -real(derivative);

                      // Move to next source direction.
                      mix.forward(1);
                    } // Source directions.

                    // Compute the residual.
                    dcomplex residual(data[tg][cr]);
                    residual -= visibility;

                    // Update the normal equations.
                    solver.makeNorm(nPartial,
                                    &(itsDerivIndex[cr * nPartial]), &(itsdR[0]),
                                    static_cast<double>(weight[cr]),
                                    real(residual));
                    solver.makeNorm(nPartial,
                                    &(itsDerivIndex[cr * nPartial]), &(itsdI[0]),
                                    static_cast<double>(weight[cr]),
                                    imag(residual));

                    // Move to next target direction.
                    mix.backward(1, nDirection);
                    mix.forward(0);
                  } // Target directions.

                  // Reset cursor to the start of the correlation.
                  mix.backward(0, nDirection);
                }

                // Move to the next correlation.
                mix.forward(2);
              } // Correlations.

              // Move to the next channel.
              mix.backward(2, 4);
              mix.forward(3);

              for (size_t dr=0; dr<nDirection; ++dr) {
                model[dr].forward(1);
                data[dr].forward(1);
              }
              flag.forward(1);
              weight.forward(1);
            } // Channels.

            // Reset cursors to the start of the baseline.
            for (size_t dr=0; dr<nDirection; ++dr) {
              model[dr].backward(1, nChannel);
              data[dr].backward(1, nChannel);
            }
            flag.backward(1, nChannel);
            weight.backward(1, nChannel);
            mix.backward(3, nChannel);
          }

          // Move cursors to the next baseline.
          for (size_t dr=0; dr<nDirection; ++dr) {
            model[dr].forward(2);
            data[dr].forward(2);
          }
          flag.forward(2);
          weight.forward(2);
          mix.forward(4);
          ++baselines;
        } // Baselines.

        // Reset all cursors for the next iteration.
        for (size_t dr=0; dr<nDirection; ++dr) {
          model[dr].backward(2, nBaseline);
          data[dr].backward(2, nBaseline);
        }
        flag.backward(2, nBaseline);
        weight.backward(2, nBaseline);
        mix.backward(4, nBaseline);
        baselines -= nBaseline;

        // Perform LSQ iteration.
        casa::uInt rank;
        bool status = solver.solveLoop(rank, unknowns, true);
        ASSERT(status);

        // Update iteration count.
        ++nIterations;
      }

      bool converged = (solver.isReady() == casa::LSQFit::SOLINCREMENT  ||
                        solver.isReady() == casa::LSQFit::DERIVLEVEL);
      return converged;
    }

  } //# namespace DPPP
} //# namespace LOFAR
