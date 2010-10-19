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

#include "MEPHeader.h"

#include <string.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace EPA_Protocol;
using namespace std;

//
// header templates
//
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
		    uint8  dstid,
		    uint8  pid,
		    uint8  regid,
		    uint16 size,
		    uint16 offset)
{
  memset(&m_fields, 0, sizeof(m_fields));
  m_fields.type = type;
  m_fields.addr.dstid = dstid;
  m_fields.addr.pid = pid;
  m_fields.addr.regid = regid;
  m_fields.offset = offset;
  m_fields.size = size;
}

void MEPHeader::set(MEPHeader::FieldsType hdrtemplate,
		    uint8  dstid,
		    uint8  type,
		    uint16 size,
		    uint16 offset)
{
  m_fields = hdrtemplate;

  if (MEPHeader::TYPE_UNSET != type) m_fields.type = type;
  m_fields.addr.dstid = dstid;
  m_fields.offset = offset;
  if (size) m_fields.size = size;
}

bool MEPHeader::isValidAck(const MEPHeader& reqhdr)
{
  /**
   * This header should be either READACK or WRITEACK,
   * have no errors and all other fields should
   * match the fields of request.
   */
  return (
    ( (READACK == this->m_fields.type) ||
      (WRITEACK == this->m_fields.type) ) &&
    (0 == this->m_fields.error) &&
    (this->m_fields.seqnr      == reqhdr.m_fields.seqnr)      &&
    (this->m_fields.addr.dstid == reqhdr.m_fields.addr.dstid) &&
    (this->m_fields.addr.pid   == reqhdr.m_fields.addr.pid)   &&
    (this->m_fields.addr.regid == reqhdr.m_fields.addr.regid) &&
    (this->m_fields.offset     == reqhdr.m_fields.offset)     &&
    (this->m_fields.size       == reqhdr.m_fields.size)
    );
}

