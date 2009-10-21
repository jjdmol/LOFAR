#include <lofar_config.h>

#include <PencilBeams.h>
#include <Common/lofar_complex.h>
#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
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

inline bool same( const float a, const float b )
{
  return abs(a-b) < TOLERANCE;
}

void test_flyseye() {
  std::vector<unsigned> stationMapping(0);
  FilteredData   in( NRSTATIONS, NRCHANNELS, NRSAMPLES );
  BeamFormedData out( NRPENCILBEAMS, NRCHANNELS, NRSAMPLES );

  assert( NRSTATIONS == NRPENCILBEAMS );

  in.allocate();
  out.allocate();

  // fill filtered data
  for( unsigned c = 0; c < NRCHANNELS; c++ ) {
    for( unsigned s = 0; s < NRSTATIONS; s++ ) {
      for( unsigned i = 0; i < NRSAMPLES; i++ ) {
        for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
          in.samples[c][s][i][p] = makefcomplex( s+1, 0 );
        }
      }
    }
  }

  // form beams
  BeamFormer f = BeamFormer( NRPENCILBEAMS, NRSTATIONS, NRCHANNELS, NRSAMPLES, CHANNELBW, stationMapping, true );
  f.mergeStations( &in );
  f.formBeams( 0, &in, &out, 0.0 );

  // check beamformed data
  for( unsigned c = 0; c < NRCHANNELS; c++ ) {
    for( unsigned i = 0; i < NRSAMPLES; i++ ) {
      for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
        for( unsigned s = 0; s < NRSTATIONS; s++ ) {
          const unsigned b = s;

          assert( out.samples[b][c][i][p] == in.samples[c][s][i][p] );
        }
      }
    }
  }
}

void test_stationmerger() {
  std::vector<unsigned> stationMapping(3);
  FilteredData   in( NRSTATIONS, NRCHANNELS, NRSAMPLES );

  in.allocate();

  // fill filtered data
  for( unsigned c = 0; c < NRCHANNELS; c++ ) {
    for( unsigned s = 0; s < NRSTATIONS; s++ ) {
      for( unsigned i = 0; i < NRSAMPLES; i++ ) {
        for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
          in.samples[c][s][i][p] = makefcomplex( s+1, 0 );
        }
      }
    }
  }

  // create mapping
  stationMapping[0] = 0;
  stationMapping[1] = 1;
  stationMapping[2] = 1;

  // form beams
  BeamFormer f = BeamFormer( NRPENCILBEAMS, NRSTATIONS, NRCHANNELS, NRSAMPLES, CHANNELBW, stationMapping, false );
  f.mergeStations( &in );

  // check merged data
  for( unsigned c = 0; c < NRCHANNELS; c++ ) {
    for( unsigned i = 0; i < NRSAMPLES; i++ ) {
      for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
        fcomplex sums[NRSTATIONS];

        for( unsigned s = 0; s < NRSTATIONS; s++ ) {
          sums[s] = makefcomplex(s+1,0);
        }
        for( unsigned s = 0; s < NRSTATIONS; s++ ) {
          if( stationMapping[s] != s ) {
            sums[stationMapping[s]] += makefcomplex( s+1, 0 );
          }
        }

        for( unsigned s = 0; s < NRSTATIONS; s++ ) {
          if( !same(real(sums[s]),real(in.samples[c][s][i][p])) 
           || !same(imag(sums[s]),imag(in.samples[c][s][i][p])) ) {
            std::cerr << in.samples[c][s][i][p] << " =/= " << sums[s] << " for station " << s << " channel " << c << " sample " << i << " pol " << p << std::endl;
            exit(1);
          }
        }
      }
    }
  }
}

void test_beamformer() {
  std::vector<unsigned> stationMapping(0);
  FilteredData   in( NRSTATIONS, NRCHANNELS, NRSAMPLES );
  BeamFormedData out( NRPENCILBEAMS, NRCHANNELS, NRSAMPLES );
  SubbandMetaData meta( NRSTATIONS, NRPENCILBEAMS );

  in.allocate();
  out.allocate();

  // fill filtered data
  for( unsigned c = 0; c < NRCHANNELS; c++ ) {
    for( unsigned s = 0; s < NRSTATIONS; s++ ) {
      for( unsigned i = 0; i < NRSAMPLES; i++ ) {
        for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
          in.samples[c][s][i][p] = makefcomplex( s+1, 0 );
        }
      }
    }
  }

  // fill weights
  for( unsigned s = 0; s < NRSTATIONS; s++ ) {
    meta.beams(s)[0].delayAtBegin = 
    meta.beams(s)[0].delayAfterEnd = 0.0;

    for( unsigned b = 1; b < NRPENCILBEAMS; b++ ) {
      meta.beams(s)[b].delayAtBegin = 
      meta.beams(s)[b].delayAfterEnd = 1.0 * s / b;
    }
  }

  // form beams
  BeamFormer f = BeamFormer( NRPENCILBEAMS, NRSTATIONS, NRCHANNELS, NRSAMPLES, CHANNELBW, stationMapping, false );
  f.mergeStations( &in );
  f.formBeams( &meta, &in, &out, CENTERFREQUENCY );

  // check beamformed data
  for( unsigned s = 0; s < NRSTATIONS; s++ ) {
    for( unsigned b = 0; b < NRPENCILBEAMS; b++ ) {
      assert( same( f.itsDelays[s][b], meta.beams(s)[b].delayAtBegin ) );
    }
  }

  const float averagingFactor = 1.0 / NRSTATIONS;
  const float factor = averagingFactor;

  for( unsigned b = 1; b < NRPENCILBEAMS; b++ ) {
    for( unsigned c = 0; c < NRCHANNELS; c++ ) {
      const double frequency = BASEFREQUENCY + c * CHANNELBW;

      for( unsigned i = 0; i < NRSAMPLES; i++ ) {
        assert( !out.flags[b].test(i) );

        for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
          fcomplex sum = makefcomplex( 0, 0 );

          for( unsigned s = 0; s < NRSTATIONS; s++ ) {
            dcomplex shift = phaseShift( frequency, meta.beams(s)[b].delayAtBegin );
            const fcomplex weight = makefcomplex(shift);

            sum += in.samples[c][s][i][p] * weight;
          }

          sum *= factor;

          if( !same(real(sum),real(out.samples[b][c][i][p])) 
           || !same(imag(sum),imag(out.samples[b][c][i][p])) ) {
            std::cerr << out.samples[b][c][i][p] << " =/= " << sum << " for beam " << b << " channel " << c << " sample " << i << " pol " << p << std::endl;
            exit(1);
          }
        }
      }
    }
  }
}

int main() {
  test_flyseye();
  test_stationmerger();
  test_beamformer();

  return 0;
}
