#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <CN_Math.h>
#include <Dedispersion.h>

#include <cassert>
#include <cstring>


#define BLOCK_SIZE	4096
#define FFT_SIZE	4096
#define DM		10
#define NR_STATIONS	64
#define NR_BEAMS	64
#define NR_CHANNELS	16


using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace LOFAR::TYPES;


void init(CN_Configuration &configuration)
{
  assert(BLOCK_SIZE % FFT_SIZE == 0);

  configuration.nrStations() = NR_STATIONS;
  configuration.flysEye() = false;
  configuration.nrPencilBeams() = NR_BEAMS;
  configuration.nrChannelsPerSubband() = NR_CHANNELS;
  configuration.nrSamplesPerIntegration() = BLOCK_SIZE;
  configuration.dedispersionFFTsize() = FFT_SIZE;
  configuration.sampleRate() = 195312.5;
  configuration.dispersionMeasure() = DM;
  configuration.refFreqs().push_back(50 * 195312.5);
}


void setTestPattern(FilteredData &filteredData)
{
  memset(&filteredData.samples[0][0][0][0], 0, filteredData.samples.num_elements() * sizeof(fcomplex));

  for (unsigned i = 0; i < BLOCK_SIZE; i ++)
    filteredData.samples[0][0][i][0] = cosisin(2 * M_PI * i * 5 / BLOCK_SIZE) /* + cosisin(2 * M_PI * i * 22 / BLOCK_SIZE) */;
}


void setTestPattern(BeamFormedData &beamFormedData)
{
  memset(&beamFormedData.samples[0][0][0][0], 0, beamFormedData.samples.num_elements() * sizeof(fcomplex));

  for (unsigned i = 0; i < BLOCK_SIZE; i ++)
    beamFormedData.samples[0][0][i][0] = cosisin(2 * M_PI * i * 5 / BLOCK_SIZE) /* + cosisin(2 * M_PI * i * 22 / BLOCK_SIZE) */;
}


void plot(const FilteredData &filteredData, float r, float g, float b)
{
  std::cout << "newcurve linetype solid linethickness 3 marktype none color " << r << ' ' << g << ' ' << b << " pts" << std::endl;

  for (unsigned i = 0; i < FFT_SIZE; i ++)
    std::cout << i << ' ' << real(filteredData.samples[0][0][i][0]) << std::endl;
}


void plot(const BeamFormedData &beamFormedData, float r, float g, float b)
{
  std::cout << "newcurve linetype solid linethickness 3 marktype none color " << r << ' ' << g << ' ' << b << " pts" << std::endl;

  for (unsigned i = 0; i < FFT_SIZE; i ++)
    std::cout << i << ' ' << real(beamFormedData.samples[0][0][i][0]) << std::endl;
}


int main()
{
  INIT_LOGGER_WITH_SYSINFO("tDedispersion");

  CN_Configuration configuration;
  init(configuration);

#if 0
  BeamFormedData beamFormedData(NR_BEAMS, NR_CHANNELS, BLOCK_SIZE);
  beamFormedData.allocate();
  setTestPattern(beamFormedData);

  std::cout << "newgraph xaxis size 7 yaxis size 7" << std::endl;
  plot(beamFormedData, 1, 0, 0);

  std::vector<unsigned> subbands(1, 50);
  DedispersionAfterBeamForming dedispersion(configuration, &beamFormedData, subbands);

  NSTimer timer("dedisperse total", true, true);
  timer.start();
  dedispersion.dedisperse(&beamFormedData, 50);
  timer.stop();

  plot(beamFormedData, 0, 0, 1);
#else
  FilteredData filteredData(NR_STATIONS, NR_CHANNELS, BLOCK_SIZE);
  filteredData.allocate();
  setTestPattern(filteredData);

  std::cout << "newgraph xaxis size 7 yaxis size 7" << std::endl;
  plot(filteredData, 1, 0, 0);

  std::vector<unsigned> subbands(1, 50);
  DedispersionBeforeBeamForming dedispersion(configuration, &filteredData, subbands);

  NSTimer timer("dedisperse total", true, true);
  timer.start();
  dedispersion.dedisperse(&filteredData, 50);
  timer.stop();

  plot(filteredData, 0, 0, 1);
#endif

  return 0;
}
