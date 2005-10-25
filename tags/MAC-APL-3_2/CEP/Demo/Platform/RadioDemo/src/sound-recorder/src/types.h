// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include <sys/types.h>

typedef u_int16_t    chnl_t;		// max = 4
typedef u_int16_t    bps_t;		// max = 64 (128 ??)
typedef u_int16_t    smpl_t;		// max = 48000
typedef u_int16_t    wavfmt_t;		// max = 0xFFFF
typedef int          cdtrck_t;		// linux base-type
typedef int          cdfmt_t;		// linux base-type
typedef int          cdindx_t;		// linux base-type
typedef u_int32_t    discid_t;		// cddb disc ID
