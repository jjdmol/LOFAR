#ifndef TFC_CONFIG_H
#define TFC_CONFIG_H

/* This is included by C++ and assembly files.  Do not put anything but
   constants here! */

#define NR_STATIONS		   37
#define NR_POLARIZATIONS	   2
#define NR_SUB_CHANNELS		   256
#define NR_TAPS			   16
#define NR_STATION_SAMPLES	   (49 * NR_TAPS * NR_SUB_CHANNELS)
#define NR_CORRELATORS_PER_FILTER  4

#define NR_CHANNELS_PER_CORRELATOR (NR_SUB_CHANNELS / NR_CORRELATORS_PER_FILTER)
#define NR_SAMPLES_PER_INTEGRATION (NR_STATION_SAMPLES / NR_SUB_CHANNELS)
#define NR_BASELINES		   (NR_STATIONS * (NR_STATIONS + 1) / 2)

#endif
