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

#ifndef ZOID_API_H
#define ZOID_API_H

#include <stdlib.h>

struct dispatch_array
{
    void* function_ptr;
    void* (*userbuf_allocate_cb)(int len);
};

#define __zoid_0c_in_buf(start, current) \
    ((start) - sizeof(int) + \
        ((current) - ((start) - sizeof(int)) + 239) / 240 * 240)

#define __zoid_0c_buf_align(size) \
    (((size) + 239) / 240 * 240)

#define __zoid_0c_out_buf_hdr1 sizeof(int)

#define __zoid_0c_out_buf_hdr2 (2 * sizeof(int))

#define __zoid_0c_out_buf_ftr(_i, _cnt) ((_i) < (_cnt) - 1 ? sizeof(int) : 0)

/* CNK-side calls.  */
int __zoid_submit_command(char* buffer, int cmd_len, int max_res_len,
			  int userbuf, const void* arr2d, const void* arr,
			  int arr_cnt, int arr_el_size, void* out_arr);
int __zoid_error(void);
int __zoid_excessive_size(void);

/* Daemon-side calls.  */
void __zoid_register_functions(int header_id,
			       struct dispatch_array* dispatch_array,
			       int array_size, void (*init_func)(int),
			       void (*fini_func)(void));
int __zoid_calling_process_id(void);
void __zoid_register_userbuf(void* userbuf,
			     void (*callback)(void* userbuf, void* priv),
			     void* priv);
int __zoid_send_output(int pid, int fd, const char* buffer, int len);

/* Both CNK- and daemon-side calls.  */
void* __zoid_allocate_buffer(size_t size);
void __zoid_release_buffer(void* buffer);

void *__zoid_alloc(size_t size);
void __zoid_free(void* addr);
#endif
