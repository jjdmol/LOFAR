/* Please note that this file is shared between ZOID and GLIBC!  */

/****************************************************************************/
/* ZEPTOOS:zepto-info */
/*     This file is part of ZeptoOS: The Small Linux for Big Computers.
 *     See www.mcs.anl.gov/zeptoos for more information.
 */
/* ZEPTOOS:zepto-info */
/* */
/* ZEPTOOS:zepto-fillin */
/*     $Id$
 *     ZeptoOS_Version: 1.2
 *     ZeptoOS_Heredity: FOSS_ORIG
 *     ZeptoOS_License: GPL
 */
/* ZEPTOOS:zepto-fillin */
/* */
/* ZEPTOOS:zepto-gpl */
/*      Copyright: Argonne National Laboratory, Department of Energy,
 *                 and UChicago Argonne, LLC.  2004, 2005, 2006, 2007
 *      ZeptoOS License: GPL
 * 
 *      This software is free.  See the file ZeptoOS/misc/license.GPL
 *      for complete details on your rights to copy, modify, and use this
 *      software.
 */
/* ZEPTOOS:zepto-gpl */
/****************************************************************************/

#ifndef BGL_H
#define BGL_H

#include <inttypes.h>

/* Alignment required by the double hammer.  */
#define QUAD_ALIGN __attribute__((aligned(16)))

/* Double hammer's unit of operation.  */
typedef struct
{
    unsigned int w0;
    unsigned int w1;
    unsigned int w2;
    unsigned int w3;
}
BGLQuad QUAD_ALIGN;

#define TREE_PACKET_SIZE 256
#define TREE_DATA_SIZE 240
/* Rounds up size to the multiple of *data* size of the packet, i.e. 240 bytes.
   Used to calculate sizes of buffers, because we always read complete packets
   from the network.  */
#define TREE_BUFFER_ROUNDUP(s) (((s) + TREE_DATA_SIZE - 1) / TREE_DATA_SIZE * \
			   TREE_DATA_SIZE)

/* 4-byte hardware header sent along with the 256-byte packet on the
   tree/collective network.  */

/* Version for point-to-point communication.  */
struct BGLTreePacketP2PHardHeader
{
    unsigned pclass:4;
    unsigned p2p:1;
    unsigned irq:1;
    unsigned p2paddr:24;
    unsigned chksum:2;
};

/* Version for collective communication.  */
struct BGLTreePacketGlobalHeader
{
    unsigned pclass:4;
    unsigned p2p:1;
    unsigned irq:1;
    unsigned opcode:3;
    unsigned opsize:7;
    unsigned tag:14;
    unsigned chksum:2;
};

typedef union
{
    struct BGLTreePacketP2PHardHeader p2p;
    struct BGLTreePacketGlobalHeader global;
}
BGLTreePacketHardHeader;

/* Values of "opcode" field in BGLTreePacketGlobalHeader.  */
typedef enum
{
    BGLTreeCombineOp_NONE = 0,
    BGLTreeCombineOp_OR = 1,
    BGLTreeCombineOp_AND = 2,
    BGLTreeCombineOp_XOR = 3,
    BGLTreeCombineOp_MAX = 5,
    BGLTreeCombineOp_ADD = 6
} BGLTreeCombineOp;

/* Routing class ("pclass" field of hardware headers) encompassing a single
   pset (an I/O node and its 8-64 compute nodes).  */
#define PACKET_CLASS_CIO 0


/* Tree/collective network hardware status register, for each virtual channel.
   Allows to find out if there is a message to be received, and also if
   sending a message is safe (don't send if injection counters are >= 8).  */
typedef struct
{
    unsigned injpktcnt:4;
    unsigned injquadcnt:4;
    unsigned dummy0:4;
    unsigned injhdrcnt:4;
    unsigned recpktcnt:4;
    unsigned recquadcnt:4;
    unsigned dummy1:3;
    unsigned intheader:1;
    unsigned rechdrcnt:4;
} BGLTreeStatusRegister;

/* Structure used as the software header for packets exchanged between the
   compute node processes and CIOD.  */
struct CioHeader
{
    uint8_t _cpu; /* 0 or 1 */
    uint8_t _rankInCnodes; /* obtained using BGLPersonality_rankInPset() */
    uint8_t _reserved;
    uint8_t _dataSize;
    uint16_t _treeAddress;
    uint16_t _messageCode; /* MFC_ or MTC_ constants, e.g. MTC_KILL */
    uint32_t _packetTotal;
    uint32_t _packetIndex;
};

#define PAD(x) char _pad[240 - (x)]

#define MTC_ACK 0xffff

/* A sample packet sent from CIOD to a compute node.  */
#define MTC_KILL 0xfff0
struct MTC_Kill
{
    struct S_MTC_Kill
    {
	unsigned signum;
    } s;
    PAD(sizeof(struct S_MTC_Kill));
} QUAD_ALIGN;

#define MFC_REQUESTEXIT 4
struct MFC_RequestExit
{
    struct S_MFC_RequestExit
    {
	enum Reason {EXITED = 0, SIGNALED = 1} reason;
	int status;
    } s;
    PAD(sizeof(struct S_MFC_RequestExit));
} QUAD_ALIGN;

#define MTC_REPLYEXIT MFC_REQUESTEXIT
/* No meaningful data is passed in this one.  */

#define MFC_REQUESTRESET 56
/* No meaningful data is passed in this one.  */

#define MTC_REPLYRESET MFC_REQUESTRESET
/* No meaningful data is passed in this one.  */

#define MFC_REQUESTWRITECORE 57
struct MFC_RequestWriteCore
{
    struct S_MFC_RequestWriteCore
    {
	int size;
	unsigned int offset;
    } s;
    PAD(sizeof(struct S_MFC_RequestWriteCore));
} QUAD_ALIGN;

#define MFC_REPLYWRITECORE MFC_REQUESTWRITECORE
struct MTC_ReplyWriteCore
{
    struct S_MTC_ReplyWriteCore
    {
	int rc;
	int errnum;
    } s;
    PAD(sizeof(struct S_MTC_ReplyWriteCore));
} QUAD_ALIGN;

/* Header sent with stdout/stderr from ciod to the service node.  */
struct CiodOutputHeader
{
    int fd;
    int cpu;
    int node;
    int rank;
    int len;
};

/* Prototypes of low-level communication functions from libdevices.  */
void BGLTreePacketHardHeader_InitP2P(BGLTreePacketHardHeader* header,
				     unsigned classroute, int irq, int dst);

void BGLTreePacketHardHeader_InitGlobal(BGLTreePacketHardHeader* header,
					unsigned classroute, int irq,
					BGLTreeCombineOp opfunc,
					int operandsize, int tag);

void BGLTreeFIFO_send(BGLTreePacketHardHeader* hdfifo, BGLQuad* datafifo,
		      BGLTreePacketHardHeader* hheader, BGLQuad* data);

void BGLTreeFIFO_sendH(BGLTreePacketHardHeader* hdfifo, BGLQuad* datafifo,
		       BGLTreePacketHardHeader* hheader, BGLQuad* softheader,
		       BGLQuad* data);

void BGLTreeFIFO_recv(BGLTreePacketHardHeader* hdfifo, BGLQuad* datafifo,
		      BGLTreePacketHardHeader* hheader, BGLQuad* data);

void BGLTreeFIFO_recvH(BGLTreePacketHardHeader* hdfifo, BGLQuad* datafifo,
		       BGLTreePacketHardHeader* hheader, BGLQuad* softheader,
		       BGLQuad* data);

void BGLTreeFIFO_recvF(BGLTreePacketHardHeader* hdfifo, BGLQuad* datafifo,
		       BGLTreePacketHardHeader* hheader, BGLQuad* softheader,
		       BGLQuad* (*func)(void* priv, BGLQuad* softheader),
		       void* priv);
#endif
