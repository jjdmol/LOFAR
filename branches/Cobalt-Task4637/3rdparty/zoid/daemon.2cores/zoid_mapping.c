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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <bglpersonality.h>

struct zoid_mapping
{
    char coord; /* 'x', 'y', 'z', or 't' (or '\0', if not in use).  */
    int size;
};

static struct zoid_mapping zoid_mapping[4];
static int proc_count;
static int vn_mode;

/*
 * Initialize mapping between x/y/z/t coordinates and MPI rank.
 * FIXME!  We don't support mappings provided by the user to mpirun using
 * -mapfile argument.
 */
int
__zoid_mapping_init(const char* mapping, int arg_proc_count, int arg_vn_mode,
		    const BGLPersonality* personality)
{
    int x_used = 0, y_used = 0, z_used = 0, t_used = 0;
    int i;

    proc_count = arg_proc_count;
    vn_mode = arg_vn_mode;

    /* We allow this to be larger, so that "XYZT" works in coprocessor
       mode.  */
    if (strlen(mapping) < 3 + vn_mode)
    {
	//fprintf(stderr, "Invalid BGLMPI_MAPPING\n");
	return 1;
    }

    /* Fill in the "mapping" array in the order specified by the user.  */
    for (i = 0; i < 3 + vn_mode && *mapping; i++)
    {
	switch (tolower(*mapping))
	{
	    case 'x':
		zoid_mapping[i].size = personality->xSize;
		x_used++;
		break;

	    case 'y':
		zoid_mapping[i].size = personality->ySize;
		y_used++;
		break;

	    case 'z':
		zoid_mapping[i].size = personality->zSize;
		z_used++;
		break;

	    case 't':
		if (!vn_mode)
		{
		    /* If we are not in VN mode, we should ignore T.
		       We don't flag it as an error, to support "TXYZ" in
		       coprocessor mode.
		       This is a bit of a hack...  */
		    mapping++;
		    i--;
		    continue;
		}
		zoid_mapping[i].size = 2;
		t_used++;
		break;

	    default:
		//fprintf(stderr, "Invalid BGLMPI_MAPPING\n");
		return 1;
	}

	zoid_mapping[i].coord = tolower(*mapping++);
    }

    if (x_used != 1 || y_used !=1 || z_used !=1 ||
	(vn_mode && t_used != 1))
    {
	//fprintf(stderr, "Invalid BGLMPI_MAPPING\n");
	return 1;
    }

    if (i == 3)
    {
	/* Make sure to mark unused as such (required if switching from VN
	   to CO mode).  */
	zoid_mapping[3].coord = '\0';
	zoid_mapping[3].size = 0;
    }

    return 0;
}

int
__zoid_mapping_to_coord(unsigned mpi_rank,
			unsigned *x, unsigned *y, unsigned *z, unsigned *t)
{
    int rank;
    int i;

    if (mpi_rank < 0 || mpi_rank >= proc_count)
	return -1;

    rank = 0;
    for (i = 0; i < sizeof(zoid_mapping) / sizeof(zoid_mapping[0]) &&
		zoid_mapping[i].coord; i++)
    {
	int new_coord;

	new_coord = mpi_rank % zoid_mapping[i].size;
	mpi_rank /= zoid_mapping[i].size;

	switch (zoid_mapping[i].coord)
	{
	    case 'x':
		*x = new_coord;
		break;
	    case 'y':
		*y = new_coord;
		break;
	    case 'z':
		*z = new_coord;
		break;
	    case 't':
		*t = new_coord;
		break;
	}
    }

    if (!vn_mode)
	*t = 0;

    return 0;
}

int
__zoid_mapping_to_rank(unsigned x, unsigned y, unsigned z, unsigned t,
		       unsigned *mpi_rank)
{
    int rank;
    int i, multiplier;

    if (!vn_mode && t != 0)
	return -1;

    rank = 0;
    multiplier = 1;
    for (i = 0; i < sizeof(zoid_mapping) / sizeof(zoid_mapping[0]) &&
		zoid_mapping[i].coord; i++)
    {
	int new_coord;

	switch (zoid_mapping[i].coord)
	{
	    case 'x':
		new_coord = x;
		break;
	    case 'y':
		new_coord = y;
		break;
	    case 'z':
		new_coord = z;
		break;
	    case 't':
		new_coord = t;
		break;
	    default:
		/* This is just to shut down a compiler warning.  */
		new_coord = 0;
	}

	if (new_coord < 0 || new_coord >= zoid_mapping[i].size)
	    return -1;

	rank += new_coord * multiplier;
	multiplier *= zoid_mapping[i].size;

	if (rank >= proc_count)
	    return -1;
    }

    *mpi_rank = rank;

    return 0;
}
