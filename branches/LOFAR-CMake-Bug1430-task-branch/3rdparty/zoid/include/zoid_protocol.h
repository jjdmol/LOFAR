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

#ifndef ZOID_PROTOCOL_H
#define ZOID_PROTOCOL_H

/*
  Description of the ZOID protocol:

  The protocol over the tree network is based on packets of 256 bytes.
  Each packet contains a 16-byte soft header of type "ZoidSoftHeader", leaving
  240 bytes for effective data.

  There are differences between commands and replies, and zero-copy support
  introduces additional complications.

  Commands have a simpler structure than replies.  Each command begins with
  a mixed control-data packet.  That packet starts with a 32-bit command id,
  filled in by the automatically generated command stub.  The ID is followed
  by command-specific arguments (data).  The arguments are unaligned.

  If there is a zero-copy input, it is passed in separate data packets, after
  the main command-data packets.  There is no control data for zero-copy input,
  because all necessary information is provided in the preceding command-data.
  Each chunk of zero-copy data (i.e. each line of 2-D array) begins at a new
  packet, to ease the copying from user buffers.
  The soft header "msg_length" field includes the input zero-copy data.

  Plain replies (those with no zero-copy results) are even simpler than
  commands.  They contain no command ID, only the data filled in by the
  automatically generated command stub.

  Replies with zero-copy results have the most complex structure.  They start
  with a 32-bit integer specifying offset where the zero-copy data starts.
  The offset is followed by standard stub-generated data.  Zero-copy data
  contains all control information necessary to fill in the provided target
  buffer.  The offset points at a 32-bit integer specifying the size along
  the major dimension.  If he array is 1-D, that size will be "-1".  The size
  is normally attached to the last reply-data packet, since it does not need
  to be aligned.  The major dimension size is followed by the size of the
  first minor dimension (or simply the array size for 1-D arrays).  That size
  is always in bytes.  User data lines are passed aligned to full packet
  boundary, like with zero-copy input.  However, each line, with the exception
  of the last one, is followed by a 32-bit integer specifying the byte size of
  the next line.  For replies, the soft header "msg_length" field does *not*
  include the zero-copy data; it only includes the two 32-bit integers
  following the reply-data.

  Why are zero-copy replies so much different from zero-copy commands?
  Because for commands, a single buffer can be allocated on the server for the
  whole command, including the zero-copy data.  For replies, on the other hand,
  multiple buffers must be used on the client: one provided by the ZOID
  infrastructure and one for user data.

  There is an optional support for rendezvous (for commands).  If a command
  packet contains the NEED_ACK_PACKET flag, the server must reply with
  ACK_PACKET before the rest of the command is sent.  This improves fairness,
  as nodes closer to the I/O node in the collective topology lose that
  advantage then.  If message acknowledgements are enabled, the NEED_ACK flag
  is normally set for the first packet of a long (larger than 8 packets)
  command, and the server sends out the acknowledgements one at a time, so
  that the nodes don't need to share the bandwidth.

  The "userbuf" flag introduces some comparatively minor changes to the
  protocol.  There are *no* changes for output userbuf.  For input userbuf,
  the order of packing of function arguments changes so that the argument
  specifying the array length is always the first one, right after the
  command id.  Putting it at this predictable place makes it easy to obtain
  the size of the buffer to be allocated by the callback.  In addition to that,
  the zero-copy part is not included in the softheader's msg_length field.
  Also, input userbuf commands *always* require message acknowledgements.
  Instead of NEED_ACK_PACKET, a separate INPUT_USERBUF_PACKET flag is used,
  and it is set not on the first packet of a message, but on the last packet
  of the non-userbuf portion of the message.
 */

/* Structure of the initial messages sent from compute node processes to zoid
   on VC1.  These do not have a soft header.  */
struct InitMsg
{
    int pset_cpu_rank;
    int p2p_addr;
    int pid;
    int pset_rank;
    int cpu;
    int vn_mode;
    int total_proc;
    char mapping[5];
};

/* Structure sent back from ION to CNs.  Again, no soft header.  */
struct InitMsgReply
{
    int max_buffer_size;
    int ack_threshold;
};

/* The tree/collective network soft header.  To preserve optimal alignment,
   it needs to be 16 bytes.  We might add more fields in the future, if we
   add async/collective support.  */
struct ZoidSoftHeader
{
    uint32_t zoid_id;
    uint16_t pset_cpu_rank;
    uint16_t flags;
    uint32_t msg_length;
    uint32_t errnum; /* Used in replies from zoid ION daemon only.  */
} QUAD_ALIGN;

/* Bits in "flags" above.  */
#define ZOID_SOFTHEADER_FIRST_PACKET (1 << 0)
#define ZOID_SOFTHEADER_LAST_PACKET  (1 << 1)
/* Indicates a special one-packet emergency message that can occur even
   in the middle of another message.  */
#define ZOID_SOFTHEADER_ASSERT_PACKET (1 << 2)
/* Indicates that the client will only send the rest of the message after
   the server sends it an ACK_PACKET.  */
#define ZOID_SOFTHEADER_NEED_ACK_PACKET (1 << 3)
#define ZOID_SOFTHEADER_ACK_PACKET (1 << 4)
/* Sent in the middle of input userbuf message, indicates the last packet
   of the non-userbuf part of the message.  Implies NEED_ACK_PACKET.  */
#define ZOID_SOFTHEADER_INPUT_USERBUF_PACKET (1 << 5)

/* Special-purpose command to inform the zoid daemon that a process is
   about to terminate.  */
#define ZOID_TERMINATING_ID ~0

/* Unique ZOID identifier.  */
#define ZOID_ID ('Z' << 24 | 'O' << 16 | 'I' << 8 | 'D')

#endif
