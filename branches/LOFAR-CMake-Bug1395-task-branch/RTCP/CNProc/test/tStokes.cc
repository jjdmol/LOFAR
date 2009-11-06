#include <lofar_config.h>

#include <PencilBeams.h>
#include <Stokes.h>
#include <Common/lofar_complex.h>
#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/StokesData.h>
#include <vector>

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace LOFAR::TYPES;

#define NRSTATIONS              3
#define NRPENCILBEAMS           3

#define NRCHANNELS              256
#define NRSAMPLES               128 // keep computation time short, 128 is minimum (see PencilBeams.cc)

#define CENTERFREQUENCY         (80.0e6)
#define BASEFREQUENCY           (CENTERFREQUENCY - (NRCHANNELS/2)*CHANNELBW)
#define CHANNELBW               (1.0*200e6/1024/NRCHANNELS)

#define TOLERANCE               1e-6

inline dcomplex phaseShift( const double frequency, const double delay )
{
  const double phaseShift = delay * frequency;
  const double phi = -2 * M_PI * phaseShift;
  return cosisin(phi);
}

inline double sqr( const double x ) {
  return x * x;
}

inline bool same( const float a, const float b )
{
  return abs(a-b) < TOLERANCE;
}

void test_incoherent_stokes( unsigned NRSTOKES, unsigned INTEGRATION ) {
  assert( INTEGRATION == 1 ); // no INTEGRATION supported yet

  std::vector<unsigned> stationMapping(NRSTATIONS);
  FilteredData   in( NRSTATIONS, NRCHANNELS, NRSAMPLES );
  StokesData out( false, NRSTOKES, NRPENCILBEAMS, NRCHANNELS, NRSAMPLES, INTEGRATION );

  in.allocate();
  out.allocate();

  // fill
  for( unsigned c = 0; c < NRCHANNELS; c++ ) {
    for( unsigned s = 0; s < NRSTATIONS; s++ ) {
      for( unsigned i = 0; i < NRSAMPLES; i++ ) {
        for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
          in.samples[c][s][i][p] = makefcomplex( s+1, s );
        }
      }
    }
  }

  for( unsigned s = 0; s < NRSTATIONS; s++ ) {
    stationMapping[s] = s;
  }

  // calculate
  Stokes     s = Stokes( NRSTOKES, NRCHANNELS, NRSAMPLES, INTEGRATION );
  s.calculateIncoherent( &in, &out, stationMapping );

  // check
    for( unsigned c = 0; c < NRCHANNELS; c++ ) {
      for( unsigned i = 0; i < NRSAMPLES; i++ ) {
        assert( !out.flags[0].test(i) );
        float stokesI = 0, stokesQ = 0, stokesU = 0, stokesV = 0;

        for( unsigned s = 0; s < NRSTATIONS; s++ ) {
          fcomplex sums[NR_POLARIZATIONS];

          for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
            sums[p] += in.samples[c][s][i][p];
          }

          double powerX = sqr( real(sums[0]) ) + sqr( imag(sums[0]) );
          double powerY = sqr( real(sums[1]) ) + sqr( imag(sums[1]) );

          if( NRSTOKES == 1 ) {
            stokesI += powerX + powerY;
          } else {
            assert( NRSTOKES == 4 );

            stokesI += powerX + powerY;
            stokesQ += powerX - powerY;
            stokesU += real( sums[0] * conj( sums[1] ) );
            stokesV += imag( sums[0] * conj( sums[1] ) );
          }
       }

       stokesI /= NRSTATIONS;
       stokesQ /= NRSTATIONS;
       stokesU /= NRSTATIONS;
       stokesV /= NRSTATIONS;

       if( !same(stokesI,out.samples[0][c][i][0]) )  {
         std::cerr << "StokesI: " << out.samples[0][c][i][0] << " =/= " << stokesI << " for channel " << c << " sample " << i << std::endl;
         exit(1);
       }

       if( NRSTOKES == 4 ) {
         if( !same(stokesQ,out.samples[0][c][i][1]) )  {
           std::cerr << "StokesQ: " << out.samples[0][c][i][1] << " =/= " << stokesQ << " for channel " << c << " sample " << i << std::endl;
           exit(1);
         }
         if( !same(stokesU,out.samples[0][c][i][2]) )  {
           std::cerr << "StokesU: " << out.samples[0][c][i][2] << " =/= " << stokesU << " for channel " << c << " sample " << i << std::endl;
           exit(1);
         }
         if( !same(stokesV,out.samples[0][c][i][3]) )  {
           std::cerr << "StokesV: " << out.samples[0][c][i][3] << " =/= " << stokesV << " for channel " << c << " sample " << i << std::endl;
           exit(1);
         }
       }
    }
  }
}

void test_coherent_stokes( unsigned NRSTOKES, unsigned INTEGRATION ) {
  assert( INTEGRATION == 1 ); // no INTEGRATION supported yet

  BeamFormedData in( NRPENCILBEAMS, NRCHANNELS, NRSAMPLES );
  StokesData out( true, NRSTOKES, NRPENCILBEAMS, NRCHANNELS, NRSAMPLES, INTEGRATION );

  in.allocate();
  out.allocate();

  // fill
  for( unsigned b = 0; b < NRPENCILBEAMS; b++ ) {
    for( unsigned c = 0; c < NRCHANNELS; c++ ) {
      for( unsigned i = 0; i < NRSAMPLES; i++ ) {
        for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
          in.samples[b][c][i][p] = makefcomplex( b+1, b );
        }
      }
    }
  }

  // calculate
  Stokes     s = Stokes( NRSTOKES, NRCHANNELS, NRSAMPLES, INTEGRATION );
  s.calculateCoherent( &in, &out, NRPENCILBEAMS );

  // check
  for( unsigned b = 0; b < NRPENCILBEAMS; b++ ) {
    for( unsigned c = 0; c < NRCHANNELS; c++ ) {
      for( unsigned i = 0; i < NRSAMPLES; i++ ) {
        assert( !out.flags[b].test(i) );

        fcomplex sums[NR_POLARIZATIONS];

        for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
          sums[p] = in.samples[b][c][i][p];
        }

        double powerX = sqr( real(sums[0]) ) + sqr( imag(sums[0]) );
        double powerY = sqr( real(sums[1]) ) + sqr( imag(sums[1]) );

        if( NRSTOKES == 1 ) {
          float stokesI = powerX + powerY;

          if( !same(stokesI,out.samples[b][c][i][0]) )  {
            std::cerr << "StokesI: " << out.samples[b][c][i][0] << " =/= " << stokesI << " for beam " << b << " channel " << c << " sample " << i << std::endl;
            exit(1);
          }
        } else {
          assert( NRSTOKES == 4 );

          float stokesI = powerX + powerY;
          float stokesQ = powerX - powerY;
          float stokesU = real( sums[0] * conj( sums[1] ) );
          float stokesV = imag( sums[0] * conj( sums[1] ) );

          if( !same(stokesI,out.samples[b][c][i][0]) )  {
            std::cerr << "StokesI: " << out.samples[b][c][i][0] << " =/= " << stokesI << " for beam " << b << " channel " << c << " sample " << i << std::endl;
            exit(1);
          }
          if( !same(stokesQ,out.samples[b][c][i][1]) )  {
            std::cerr << "StokesQ: " << out.samples[b][c][i][1] << " =/= " << stokesQ << " for beam " << b << " channel " << c << " sample " << i << std::endl;
            exit(1);
          }
          if( !same(stokesU,out.samples[b][c][i][2]) )  {
            std::cerr << "StokesU: " << out.samples[b][c][i][2] << " =/= " << stokesU << " for beam " << b << " channel " << c << " sample " << i << std::endl;
            exit(1);
          }
          if( !same(stokesV,out.samples[b][c][i][3]) )  {
            std::cerr << "StokesV: " << out.samples[b][c][i][3] << " =/= " << stokesV << " for beam " << b << " channel " << c << " sample " << i << std::endl;
            exit(1);
          }
        }
      }
    }
  }
}

int main() {
  test_incoherent_stokes(1,1);
  test_incoherent_stokes(4,1);

  test_coherent_stokes(1,1);
  test_coherent_stokes(4,1);

  return 0;
}
