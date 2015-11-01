/*
 * lofar_scheduler.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11575 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-04-03 12:48:43 +0000 (Thu, 03 Apr 2014) $
 * First creation : Feb 20, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/lofar_scheduler.h $
 *
 */

#ifndef LOFAR_SCHEDULER_H_
#define LOFAR_SCHEDULER_H_

#define CLEANUP_VERSION "v2.2"

#define HAS_SAS_CONNECTION
#define DEBUG_SCHEDULER

// undefine to compile away the debug output:
//#define QT_NO_DEBUG_OUTPUT

#include <vector>
#include <map>
#include <string>
#include <QString>
#include <limits>

#define FILE_WRITE_VERSION 10

#define J2000_EPOCH 2451545
// pi
#define PI	3.14159265358979323846264338327950288419716939937510
#define PI_DIV2  1.57079632679489661923132169163975144209858469968755
#define TWO_PI	6.28318530717958647692528676655900576839433879875021

#define RAD2GRAD	57.29577951308232087679815481410517033240547246656432 // 180/pi
#define GRAD2RAD	0.01745329251994329576923690768488612713442871888542  // pi/180
#define SINSUNRISESETEL -1.22170008352471688802e-2
#define DU_LST_LINEAR 24.06570982441908
#define DU_LST_CONST 18.697374558336
#define LST_DU_LINEAR 4.15528975427151E-02

// the file name masks and directory masks for the different output data products (data types)
#define NAMEMASK_COHERENT_STOKES 		"L${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.h5"
#define NAMEMASK_INCOHERENT_STOKES		"L${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.h5"
#define NAMEMASK_CORRELATED				"L${OBSID}_SAP${SAP}_SB${SUBBAND}_uv.MS"
#define NAMEMASK_INSTRUMENT_MODEL		"L${OBSID}_SAP${SAP}_SB${SUBBAND}_inst.INST"
#define NAMEMASK_PULSAR                 "L${OBSID}_SAP${SAP}_B${BEAM}_bf.tar.gz"
#define NAMEMASK_SKY_IMAGE				"L${OBSID}_SBG${SBG}_sky.h5"
#define NAMEMASK_COHERENT_STOKES_TEST 	"T${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.h5"
#define NAMEMASK_INCOHERENT_STOKES_TEST	"T${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.h5"
#define NAMEMASK_CORRELATED_TEST		"T${OBSID}_SAP${SAP}_SB${SUBBAND}_uv.MS"
#define NAMEMASK_INSTRUMENT_MODEL_TEST	"T${OBSID}_SAP${SAP}_SB${SUBBAND}_inst.INST"
#define NAMEMASK_PULSAR_TEST            "T${OBSID}_SAP${SAP}_B${BEAM}_bf.tar.gz"
#define NAMEMASK_SKY_IMAGE_TEST			"T${OBSID}_SBG${SBG}_sky.IM"
#define NAMEMASK_TRIGGER_TEST			"T${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.trigger"
#define DIRMASK_COHERENT_STOKES			"L${OBSID}"
#define DIRMASK_INCOHERENT_STOKES		"L${OBSID}"
#define DIRMASK_CORRELATED				"L${OBSID}"
#define DIRMASK_TRIGGER					"L${OBSID}"
#define DIRMASK_INSTRUMENT_MODEL		"L${OBSID}"
#define DIRMASK_PULSAR          		"L${OBSID}"
#define DIRMASK_SKY_IMAGE				"L${OBSID}"
#define DIRMASK_COHERENT_STOKES_TEST	"L${OBSID}"
#define DIRMASK_INCOHERENT_STOKES_TEST	"L${OBSID}"
#define DIRMASK_CORRELATED_TEST			"L${OBSID}"
#define DIRMASK_TRIGGER_TEST			"L${OBSID}"
#define DIRMASK_INSTRUMENT_MODEL_TEST	"L${OBSID}"
#define DIRMASK_PULSAR_TEST        		"L${OBSID}"
#define DIRMASK_SKY_IMAGE_TEST			"L${OBSID}"

enum dataProductTypes {
    _BEGIN_DATA_PRODUCTS_ENUM_,
    DP_CORRELATED_UV = _BEGIN_DATA_PRODUCTS_ENUM_,
    DP_COHERENT_STOKES,
    DP_INCOHERENT_STOKES,
    DP_INSTRUMENT_MODEL,
    DP_PULSAR,
    DP_SKY_IMAGE,
    DP_UNKNOWN_TYPE,
    _END_DATA_PRODUCTS_ENUM_
};
#define NR_DATA_PRODUCT_TYPES	_END_DATA_PRODUCTS_ENUM_

extern const char * DATA_PRODUCTS[NR_DATA_PRODUCT_TYPES];

class campaignInfo {
public:
	unsigned id;
	std::string name;
	std::string title;
	std::string PriInvestigator;
	std::string CoInvestigator;
	std::string contact;
};

typedef std::map<std::string ,campaignInfo> campaignMap;

#define PROGRAM_DEFAULT_SETTINGS_FILENAME ".cleanup_default_settings.set"
#define PROGRAM_PREFERENCES_FILENAME ".cleanup_preferences.pre"

#define MAX_UNSIGNED std::numeric_limits<unsigned int>::max()

void debugInfo( std::string szTypes, ... ); // debug info
void debugWarn( std::string szTypes, ... ); // debug warning
void debugErr(  std::string szTypes, ...); // debug error


#endif /* LOFAR_SCHEDULER_H_ */
