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

#ifndef ZOID_MAPPING_H
#define ZOID_MAPPING_H

#ifdef __bglpersonality_h__
int __zoid_mapping_init(const char* mapping, int arg_proc_count,
			int arg_vn_mode, const BGLPersonality* personality);
#endif
int __zoid_mapping_to_coord(unsigned mpi_rank, unsigned *x, unsigned *y,
			    unsigned *z, unsigned *t);

int __zoid_mapping_to_rank(unsigned x, unsigned y, unsigned z, unsigned t,
			   unsigned *mpi_rank);
#endif
