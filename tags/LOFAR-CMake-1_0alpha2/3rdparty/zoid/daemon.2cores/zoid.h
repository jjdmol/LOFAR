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

#ifndef ZOIDD_H
#define ZOIDD_H

#include "bgl.h"
#include "zoid_protocol.h"
#include "bglco.h"

#include <pthread.h>

#undef L1_CONSISTENCY_IN_SOFTWARE

struct CNProc
{
    /* Info data.  */
    int pid;
    uint16_t p2p_addr;
    uint8_t pset_rank;
    uint8_t cpu;

    /* Message currently being received.  */
    int msg_length;
    struct zoid_buffer* buffer;
    char* current_buf;

    /* Termination info */
    char status;
} QUAD_ALIGN;

#define PROC_STATUS_RUNNING 0
#define PROC_STATUS_EXIT 1
#define PROC_STATUS_EXIT_ABNORMAL 2

struct thread_specific_data
{
    int calling_process_id;
    int errnum;
    int last_excessive_size;

    /* Support for output userbuf.  */
    void* userbuf;
    /* The following two fields are only valid if userbuf != NULL.  */
    void (*userbuf_cb)(void* userbuf, void* priv);
    void* userbuf_priv;
};

/* WARNING!  When modifying this structure, ensure that softheader and data
   remain quad-aligned!  */
struct zoid_buffer
{
    struct zoid_buffer* next;
    volatile unsigned int size;

    /* Declared and actual result length.  */
    int result_len;
    int total_len;

    /* Error indicators.  */
    int errnum;
    int excessive_size;

    /* Support for output userbuf.  */
    void* userbuf_out;
    void (*userbuf_out_cb)(void* userbuf, void* priv);
    void* userbuf_out_priv;

    /* Support for input userbuf.  */
    char* userbuf_in;
    int userbuf_in_len;

    /* Support for input flow control.  */
    char ack_sent;

    /* Quad-alignment required for the remaining fields.  */
    char pad[3];

    struct ZoidSoftHeader softheader;

    char data[0]; /* Variable size */
} QUAD_ALIGN;

struct zoid_dispatch_entry
{
    struct zoid_dispatch_entry* next;
    int header_id;
    struct dispatch_array* dispatch_array;
    int array_size;
    void (*init_func)(int pset_proc_count);
    void (*fini_func)(void);
};

typedef struct lock_pair_s {
    pthread_mutex_t pt_mutex;
    BGL_Mutex *hw_mutex;
} lock_pair;

typedef struct zoid_buf_pipe_s {
    struct zoid_buffer *volatile first; /* volatile added by John */
    struct zoid_buffer *volatile last; /* volatile added by John */
    lock_pair *locks;
} zoid_buf_pipe;

struct zoid_buffer* get_zoid_buffer(void* buffer);

void* worker_thread_body(void* arg);
void* ciod_thread_body(void* arg);
void bglco_loop(void);

void cleanup_traffic(void);

void allocater_init(void);

extern void *sram;
extern void *lockbox;

extern void* vc0;
extern struct zoid_buffer* packet_buffer;
extern struct CNProc* cn_procs;
extern int pset_size;
extern int vn_mode;
extern int pset_proc_count;
extern int max_buffer_size_1, max_buffer_size_2;
extern int pending_exit_requests;
extern int abnormal_msg_received;

extern struct zoid_dispatch_entry* dispatch_entries;

extern pthread_key_t thread_specific_key;
extern pthread_mutex_t ack_queue_mutex;
extern pthread_mutex_t output_mutex;

extern int pset_rank_mapping[128];
extern int pset_rank_mapping_rev[128];

extern int ciod_control_socket;
extern int ciod_streams_socket;
extern int sent_kill_packet;

extern zoid_buf_pipe *recv_queue;
extern lock_pair recv_queue_locks;

extern zoid_buf_pipe *send_queue;
extern lock_pair send_queue_locks;

extern zoid_buf_pipe *high_priority_send_queue;
extern lock_pair high_priority_send_queue_locks;

extern zoid_buf_pipe *ack_queue;
extern lock_pair ack_queue_locks;

extern lock_pair pending_exit_locks;
extern lock_pair tree_locks;

extern void init_lock_pair(lock_pair *);
extern inline void enter_critical_section(lock_pair*);
extern inline void leave_critical_section(lock_pair*);

extern void init_zoid_buf_pipe(zoid_buf_pipe*, lock_pair*);
extern void enqueue_zoid_buf(zoid_buf_pipe*, struct zoid_buffer*);
extern struct zoid_buffer *dequeue_zoid_buf(zoid_buf_pipe*);


#if defined L1_CONSISTENCY_IN_SOFTWARE
extern char l1flusher[32768];
extern void flush_L1_all();
extern void flush_L1_region(void *addr, unsigned int size);
extern void flush_zoid_buf(struct zoid_buffer *buffer);
#endif

#endif
