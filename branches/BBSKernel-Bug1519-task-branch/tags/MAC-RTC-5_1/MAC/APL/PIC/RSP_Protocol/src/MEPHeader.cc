//#  MEPHeader.h: implementation of the MEPHeader class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/MEPHeader.h>

#include <string.h>

using namespace LOFAR;
using namespace EPA_Protocol;
using namespace std;

//
// header templates
//
const MEPHeader::FieldsType MEPHeader::RSR_STATUS_HDR        = { READ,  0, 0, { DST_RSP, RSR, RSR_STATUS  }, 0, RSR_STATUS_SIZE,  0, 0 };
const MEPHeader::FieldsType MEPHeader::RSR_VERSION_HDR       = { READ,  0, 0, { DST_RSP, RSR, RSR_VERSION }, 0, RSR_VERSION_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::RSU_FLASHRW_HDR       = { WRITE, 0, 0, { DST_RSP, RSU, RSU_FLASHRW     }, 0, RSU_FLASHRW_SIZE,     0, 0 };
const MEPHeader::FieldsType MEPHeader::RSU_FLASHERASE_HDR    = { WRITE, 0, 0, { DST_RSP, RSU, RSU_FLASHERASE  }, 0, RSU_FLASHERASE_SIZE,  0, 0 };
const MEPHeader::FieldsType MEPHeader::RSU_RECONFIGURE_HDR   = { WRITE, 0, 0, { DST_RSP, RSU, RSU_RECONFIGURE }, 0, RSU_RECONFIGURE_SIZE, 0, 0 };
const MEPHeader::FieldsType MEPHeader::RSU_RESET_HDR         = { WRITE, 0, 0, { DST_RSP, RSU, RSU_RESET       }, 0, RSU_RESET_SIZE,       0, 0 };

const MEPHeader::FieldsType MEPHeader::DIAG_WGX_HDR          = { WRITE, 0, 0, { DST_BLP0, DIAG, DIAG_WGX      }, 0, DIAG_WGX_SIZE,      0, 0 };
const MEPHeader::FieldsType MEPHeader::DIAG_WGY_HDR          = { WRITE, 0, 0, { DST_BLP0, DIAG, DIAG_WGY      }, 0, DIAG_WGY_SIZE,      0, 0 };
const MEPHeader::FieldsType MEPHeader::DIAG_WGXWAVE_HDR      = { WRITE, 0, 0, { DST_BLP0, DIAG, DIAG_WGXWAVE  }, 0, DIAG_WGXWAVE_SIZE,  0, 0 };
const MEPHeader::FieldsType MEPHeader::DIAG_WGYWAVE_HDR      = { WRITE, 0, 0, { DST_BLP0, DIAG, DIAG_WGYWAVE  }, 0, DIAG_WGYWAVE_SIZE,  0, 0 };
const MEPHeader::FieldsType MEPHeader::DIAG_BYPASS_HDR       = { WRITE, 0, 0, { DST_BLP0, DIAG, DIAG_BYPASS   }, 0, DIAG_BYPASS_SIZE,   0, 0 };
const MEPHeader::FieldsType MEPHeader::DIAG_RESULTS_HDR      = { READ,  0, 0, { DST_BLP0, DIAG, DIAG_RESULTS  }, 0, DIAG_RESULTS_SIZE,  0, 0 };
const MEPHeader::FieldsType MEPHeader::DIAG_SELFTEST_HDR     = { WRITE, 0, 0, { DST_RSP , DIAG, DIAG_SELFTEST }, 0, DIAG_SELFTEST_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::SS_SELECT_HDR         = { WRITE, 0, 0, { DST_BLP0, SS, SS_SELECT }, 0, SS_SELECT_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::BF_XROUT_HDR          = { WRITE, 0, 0, { DST_BLP0, BF, BF_XROUT }, 0, BF_XROUT_SIZE, 0, 0 };
const MEPHeader::FieldsType MEPHeader::BF_XIOUT_HDR          = { WRITE, 0, 0, { DST_BLP0, BF, BF_XIOUT }, 0, BF_XIOUT_SIZE, 0, 0 };
const MEPHeader::FieldsType MEPHeader::BF_YROUT_HDR          = { WRITE, 0, 0, { DST_BLP0, BF, BF_YROUT }, 0, BF_YROUT_SIZE, 0, 0 };
const MEPHeader::FieldsType MEPHeader::BF_YIOUT_HDR          = { WRITE, 0, 0, { DST_BLP0, BF, BF_YIOUT }, 0, BF_YIOUT_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::BST_POWER_HDR         = { READ,  0, 0, { DST_RSP, BST, BST_POWER }, 0, BST_POWER_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::SST_POWER_HDR         = { READ,  0, 0, { DST_BLP0, SST, SST_POWER }, 0, SST_POWER_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::RCU_SETTINGS_HDR      = { WRITE, 0, 0, { DST_BLP0, RCU, RCU_SETTINGS  }, 0, RCU_SETTINGS_SIZE,  0, 0 };
const MEPHeader::FieldsType MEPHeader::RCU_PROTOCOLX_HDR     = { WRITE, 0, 0, { DST_BLP0, RCU, RCU_PROTOCOLX }, 0, RCU_PROTOCOL_SIZE, 0, 0 };
const MEPHeader::FieldsType MEPHeader::RCU_RESULTX_HDR       = { READ,  0, 0, { DST_BLP0, RCU, RCU_RESULTX   }, 0, RCU_RESULT_SIZE,   0, 0 };
const MEPHeader::FieldsType MEPHeader::RCU_PROTOCOLY_HDR     = { WRITE, 0, 0, { DST_BLP0, RCU, RCU_PROTOCOLY }, 0, RCU_PROTOCOL_SIZE, 0, 0 };
const MEPHeader::FieldsType MEPHeader::RCU_RESULTY_HDR       = { READ,  0, 0, { DST_BLP0, RCU, RCU_RESULTY   }, 0, RCU_RESULT_SIZE,   0, 0 };

const MEPHeader::FieldsType MEPHeader::CR_CONTROL_HDR        = { WRITE, 0, 0, { DST_RSP, CR, CR_CONTROL }, 0, CR_CONTROL_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::XST_STATS_HDR         = { READ,  0, 0, { DST_RSP, XST, XST_STATS }, 0, XST_STATS_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::CDO_SETTINGS_HDR      = { WRITE, 0, 0, { DST_RSP, CDO, CDO_SETTINGS }, 0, CDO_SETTINGS_SIZE, 0, 0 };
const MEPHeader::FieldsType MEPHeader::CDO_HEADER_HDR        = { WRITE, 0, 0, { DST_RSP, CDO, CDO_HEADER   }, 0, CDO_HEADER_SIZE,   0, 0 };

const MEPHeader::FieldsType MEPHeader::BS_NOF_SAMPLES_PER_SYNC_HDR = { WRITE, 0, 0, { DST_BLP0, BS, BS_NOF_SAMPLES_PER_SYNC }, 0, BS_NOF_SAMPLES_PER_SYNC_SIZE, 0, 0 };

const MEPHeader::FieldsType MEPHeader::TDS_PROTOCOL_HDR      = { WRITE, 0, 0, { DST_RSP, TDS, TDS_PROTOCOL }, 0, TDS_PROTOCOL_SIZE, 0, 0 };
const MEPHeader::FieldsType MEPHeader::TDS_RESULT_HDR        = { READ,  0, 0, { DST_RSP, TDS, TDS_RESULT   }, 0, TDS_RESULT_SIZE,   0, 0 };

const MEPHeader::FieldsType MEPHeader::TBB_CONTROL_HDR       = { WRITE, 0, 0, { DST_RSP, TBB, TBB_CONTROL }, 0, TBB_CONTROL_SIZE, 0, 0 };

/* OLD REGISTER

const MEPHeader::FieldsType MEPHeader::RSR_STATUS_HDR    = { READ,  0, 0, { DST_RSP, RSR, RSR_STATUS,    0 }, 0, RSR_STATUS_SIZE    };
const MEPHeader::FieldsType MEPHeader::RSR_VERSION_HDR   = { READ,  0, 0, { DST_RSP, RSR, RSR_VERSION,   0 }, 0, RSR_VERSION_SIZE   };

const MEPHeader::FieldsType MEPHeader::TST_SELFTEST_HDR  = { WRITE, 0, 0, { DST_RSP, TST, TST_SELFTEST,  0 }, 0, TST_SELFTEST_SIZE  };

const MEPHeader::FieldsType MEPHeader::CFG_REPROGRAM_HDR = { WRITE, 0, 0, { DST_RSP, CFG, CFG_REPROGRAM, 0 }, 0, CFG_REPROGRAM_SIZE };

const MEPHeader::FieldsType MEPHeader::WG_XSETTINGS_HDR  = { WRITE, 0, 0, { DST_BLP, WG,  WG_XSETTINGS,  0 }, 0, WG_XSETTINGS_SIZE  };
const MEPHeader::FieldsType MEPHeader::WG_YSETTINGS_HDR  = { WRITE, 0, 0, { DST_BLP, WG,  WG_YSETTINGS,  0 }, 0, WG_YSETTINGS_SIZE  };
const MEPHeader::FieldsType MEPHeader::WG_XWAVE_HDR      = { WRITE, 0, 0, { DST_BLP, WG,  WG_XWAVE,      0 }, 0, WG_XWAVE_SIZE      };
const MEPHeader::FieldsType MEPHeader::WG_YWAVE_HDR      = { WRITE, 0, 0, { DST_BLP, WG,  WG_YWAVE,      0 }, 0, WG_YWAVE_SIZE      };
      
const MEPHeader::FieldsType MEPHeader::SS_SELECT_HDR     = { WRITE, 0, 0, { DST_BLP, SS,  SS_SELECT,     0 }, 0, SS_SELECT_SIZE     };

const MEPHeader::FieldsType MEPHeader::BF_XROUT_HDR      = { WRITE, 0, 0, { DST_BLP, BF,  BF_XROUT,      0 }, 0, BF_XROUT_SIZE      };
const MEPHeader::FieldsType MEPHeader::BF_XIOUT_HDR      = { WRITE, 0, 0, { DST_BLP, BF,  BF_XIOUT,      0 }, 0, BF_XIOUT_SIZE      };
const MEPHeader::FieldsType MEPHeader::BF_YROUT_HDR      = { WRITE, 0, 0, { DST_BLP, BF,  BF_YROUT,      0 }, 0, BF_YROUT_SIZE      };
const MEPHeader::FieldsType MEPHeader::BF_YIOUT_HDR      = { WRITE, 0, 0, { DST_BLP, BF,  BF_YIOUT,      0 }, 0, BF_YIOUT_SIZE      };

const MEPHeader::FieldsType MEPHeader::BST_POWER_HDR     = { READ,  0, 0, { DST_RSP, BST, BST_POWER,     0 }, 0, BST_POWER_SIZE     };

const MEPHeader::FieldsType MEPHeader::SST_POWER_HDR     = { READ,  0, 0, { DST_BLP, SST, SST_POWER,     0 }, 0, SST_POWER_SIZE     };

const MEPHeader::FieldsType MEPHeader::RCU_SETTINGS_HDR  = { WRITE, 0, 0, { DST_BLP, RCU, RCU_SETTINGS,  0 }, 0, RCU_SETTINGS_SIZE  };

const MEPHeader::FieldsType MEPHeader::CRR_SOFTRESET_HDR = { WRITE, 0, 0, { DST_RSP, CRR, CRR_SOFTRESET, 0 }, 0, CRR_SOFTRESET_SIZE };
const MEPHeader::FieldsType MEPHeader::CRR_SOFTPPS_HDR   = { WRITE, 0, 0, { DST_RSP, CRR, CRR_SOFTPPS,   0 }, 0, CRR_SOFTPPS_SIZE   };

const MEPHeader::FieldsType MEPHeader::CRB_SOFTRESET_HDR = { WRITE, 0, 0, { DST_BLP, CRB, CRB_SOFTRESET, 0 }, 0, CRB_SOFTRESET_SIZE };
const MEPHeader::FieldsType MEPHeader::CRB_SOFTPPS_HDR   = { WRITE, 0, 0, { DST_BLP, CRB, CRB_SOFTPPS,   0 }, 0, CRB_SOFTPPS_SIZE   };

const MEPHeader::FieldsType MEPHeader::CDO_SETTINGS_HDR  = { WRITE, 0, 0, { DST_RSP, CDO, CDO_SETTINGS,  0 }, 0, CDO_SETTINGS_SIZE  };
const MEPHeader::FieldsType MEPHeader::CDO_HEADER_HDR    = { WRITE, 0, 0, { DST_RSP, CDO, CDO_HEADER,    0 }, 0, CDO_HEADER_SIZE    };

const MEPHeader::FieldsType MEPHeader::XST_STATS_HDR     = { READ,  0, 0, { DST_RSP, XST, XST_STATS,     0 }, 0, XST_STATS_SIZE     };

*/

unsigned int MEPHeader::getSize()
{
  return MEPHeader::SIZE;
}

unsigned int MEPHeader::pack  (void* buffer)
{
  memcpy(buffer, &(this->m_fields), MEPHeader::SIZE);
  return MEPHeader::SIZE;
}

unsigned int MEPHeader::unpack(void *buffer)
{
  memcpy(&(this->m_fields), buffer, MEPHeader::SIZE);
  return MEPHeader::SIZE;
}

void MEPHeader::set(uint8  type,
		    uint16 dstid,
		    uint8  pid,
		    uint8  regid,
		    uint16 payload_length,
		    uint16 offset)
{
  memset(&m_fields, 0, sizeof(m_fields));
  m_fields.type           = type;

  // set frame_length depending on type of message
  switch (type) {

  case MEPHeader::READ:
  case MEPHeader::WRITEACK:
    m_fields.frame_length = MEPHeader::SIZE;
    break;

  case MEPHeader::WRITE:
  case MEPHeader::READACK:
    m_fields.frame_length = MEPHeader::SIZE + payload_length;
    break;
  }

  m_fields.addr.dstid     = dstid;
  m_fields.addr.pid       = pid;
  m_fields.addr.regid     = regid;
  m_fields.offset         = offset;
  m_fields.payload_length = payload_length;
}

void MEPHeader::set(MEPHeader::FieldsType hdrtemplate,
		    uint16 dstid,
		    uint8  type,
		    uint16 payload_length,
		    uint16 offset)
{
  memset(&m_fields, 0, sizeof(m_fields));
  m_fields = hdrtemplate;

  if (MEPHeader::TYPE_UNSET != type) m_fields.type = type;

  if (payload_length) m_fields.payload_length = payload_length;

  // set or compute the correct frame_length
  m_fields.frame_length = MEPHeader::SIZE;

  switch (m_fields.type) {

    case MEPHeader::WRITE:
    case MEPHeader::READACK:
      m_fields.frame_length += m_fields.payload_length;
      break;
  }

  m_fields.addr.dstid = dstid;
  m_fields.offset = offset;
}

bool MEPHeader::isValidAck(const MEPHeader& reqhdr)
{
  /**
   * This header should be either READACK or WRITEACK,
   * have no errors and all other fields should
   * match the fields of the request except for the
   * frame_length fields which should be just the MEPHeader::SIZE
   * if it is a WRITEACK, or SIZE + payload_length if it is a
   * READACK.
   */
  return (
    ( (READACK == this->m_fields.type && reqhdr.m_fields.type == READ) ||
      (WRITEACK == this->m_fields.type && reqhdr.m_fields.type == WRITE) ) &&
    (0 == this->m_fields.status) &&
    (((READACK  == this->m_fields.type) && (this->m_fields.frame_length == MEPHeader::SIZE + reqhdr.m_fields.payload_length)) ||
     ((WRITEACK == this->m_fields.type) && (this->m_fields.frame_length == MEPHeader::SIZE))) &&
    (this->m_fields.seqnr          == reqhdr.m_fields.seqnr)      &&
    (this->m_fields.addr.dstid     == reqhdr.m_fields.addr.dstid) &&
    (this->m_fields.addr.pid       == reqhdr.m_fields.addr.pid)   &&
    (this->m_fields.addr.regid     == reqhdr.m_fields.addr.regid) &&
    (this->m_fields.offset         == reqhdr.m_fields.offset)     &&
    (this->m_fields.payload_length == reqhdr.m_fields.payload_length)
    );
}

