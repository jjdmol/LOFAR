/* Interface to the PPS API described in RFC 2783 (March 2000)
 *
 * Copyright (c) 1999, 2001, 2004, 2006 by Ulrich Windl,
 * 	based on code by Reg Clemens <reg@dwf.com>
 *		based on code by Poul-Henning Kamp <phk@FreeBSD.org>
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.org> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ---------------------------------------------------------------------------- */

#ifndef _SYS_TIMEPPS_H_
#define _SYS_TIMEPPS_H_

/* Implementation note: the logical states ``assert'' and ``clear''
 * are implemented in terms of the chip register, i.e. ``assert''
 * means the bit is set.
 */

/*---- 3.2 New data structures ----*/
#define PPS_API_VERS_1	1		/* draft-05, dated 1999-08 */

typedef int pps_handle_t;	 	/* represents a PPS source */

typedef unsigned long pps_seq_t;	/* sequence number */

typedef struct ntp_fp {
	unsigned int	integral;
	unsigned int	fractional;
} ntp_fp_t;				/* NTP-compatible time stamp */

#include <time.h>		/* REO: 070909 Added because check of Cmake failed */

typedef union pps_timeu {
	struct timespec	tspec;
	ntp_fp_t	ntpfp;
	unsigned long	longpad[3];
} pps_timeu_t;		/* generic data type to represent time stamps */

typedef struct pps_info {
	pps_seq_t	assert_sequence;	/* seq. num. of assert event */
	pps_seq_t	clear_sequence;		/* seq. num. of clear event */
	pps_timeu_t	assert_tu;		/* time of assert event */
	pps_timeu_t	clear_tu;		/* time of clear event */
	int		current_mode;		/* current mode bits */
} pps_info_t;

#define assert_timestamp        assert_tu.tspec
#define clear_timestamp         clear_tu.tspec

#define assert_timestamp_ntpfp  assert_tu.ntpfp
#define clear_timestamp_ntpfp   clear_tu.ntpfp

typedef struct pps_params {
	int		api_version;	/* API version # */
	int		mode;		/* mode bits */
	pps_timeu_t assert_off_tu;	/* offset compensation for assert */
	pps_timeu_t clear_off_tu;	/* offset compensation for clear */
} pps_params_t;

#define assert_offset   assert_off_tu.tspec
#define clear_offset    clear_off_tu.tspec

#define assert_offset_ntpfp     assert_off_tu.ntpfp
#define clear_offset_ntpfp      clear_off_tu.ntpfp

/*---- 3.3 Mode bit definitions ----*/
/*-- Device/implementation parameters --*/
#define PPS_CAPTUREASSERT	0x01	/* capture assert events */
#define PPS_CAPTURECLEAR	0x02	/* capture clear events */
#define PPS_CAPTUREBOTH		0x03	/* capture assert and clear events */

#define PPS_OFFSETASSERT	0x10	/* apply compensation for assert ev. */
#define PPS_OFFSETCLEAR		0x20	/* apply compensation for clear ev. */

#define PPS_CANWAIT		0x100	/* Can we wait for an event? */
#define PPS_CANPOLL		0x200	/* "This bit is reserved for
                                           future use." */

/*-- Kernel actions --*/
#define PPS_ECHOASSERT		0x40	/* feed back assert event to output */
#define PPS_ECHOCLEAR		0x80	/* feed back clear event to output */

/*-- Timestamp formats --*/
#define PPS_TSFMT_TSPEC		0x1000	/* select timespec format */
#define PPS_TSFMT_NTPFP		0x2000	/* select NTP format */

/*---- 3.4.4 New functions: disciplining the kernel timebase ----*/
/*-- Kernel consumers --*/
#define PPS_KC_HARDPPS		0	/* hardpps() (or equivalent) */
#define PPS_KC_HARDPPS_PLL	1	/* hardpps() constrained to
					   use a phase-locked loop */
#define PPS_KC_HARDPPS_FLL	2	/* hardpps() constrained to
					   use a frequency-locked loop */

/*------ Here begins the implementation-specific part! ------*/
struct pps_fetch_args {
	int		tsformat;
	struct timespec	timeout;
	pps_info_t	pps_info_buf;
};

struct pps_bind_args {
	int		tsformat;	/* format of time stamps */
	int		edge;		/* selected event type */
	int		consumer;	/* selected kernel consumer */
};

/* check Documentation/ioctl-number.txt! */
#define PPS_IOC_CREATE		_IO('1', 1)
#define PPS_IOC_DESTROY		_IO('1', 2)
#define PPS_IOC_SETPARMS	_IOW('1', 3, pps_params_t)
#define PPS_IOC_GETPARMS	_IOR('1', 4, pps_params_t)
#define PPS_IOC_GETCAP		_IOR('1', 5, int)
#define PPS_IOC_FETCH		_IOWR('1', 6, struct pps_fetch_args)
#define PPS_IOC_KC_BIND		_IOW('1', 7, struct pps_bind_args)

#ifndef __KERNEL__

#include	<sys/ioctl.h>

/*---- 3.4 Functions ----*/

/* create PPS handle from file descriptor */
static __inline int time_pps_create(int filedes, pps_handle_t *handle)
{
	int error;

	error = ioctl(filedes, PPS_IOC_CREATE, 0);
	if (error < 0) {
		*handle = -1;
		return (-1);
	}
	*handle = filedes;
	return (0);
}

/* release PPS handle */
static __inline int time_pps_destroy(pps_handle_t handle)
{
	return (ioctl(handle, PPS_IOC_DESTROY, 0));
}

/* set parameters for handle */
static __inline int time_pps_setparams(pps_handle_t handle,
				       const pps_params_t *ppsparams)
{
	if (ppsparams->api_version != PPS_API_VERS_1) {
		/* This is ugly, but there was no reasonable consensus
                   in the API working group.  I require
                   ``api_version'' to be set!
		*/
		((pps_params_t *) ppsparams)->api_version = PPS_API_VERS_1;
	}

	return (ioctl(handle, PPS_IOC_SETPARMS, ppsparams));
}

/* get parameters for handle */
static __inline int time_pps_getparams(pps_handle_t handle,
				       pps_params_t *ppsparams)
{
	return (ioctl(handle, PPS_IOC_GETPARMS, ppsparams));
}

/* get capabilities for handle */
static __inline int time_pps_getcap(pps_handle_t handle, int *mode)
{
	return (ioctl(handle, PPS_IOC_GETCAP, mode));
}

/* current event for handle */
static __inline int time_pps_fetch(pps_handle_t handle, const int tsformat,
				   pps_info_t *ppsinfobuf,
				   const struct timespec *timeout)
{
	int error;
	struct pps_fetch_args arg;

	arg.tsformat = tsformat;
	if (timeout)
		arg.timeout = *timeout;
	else	/* wait forever */
		arg.timeout.tv_sec = arg.timeout.tv_nsec = -1;
	error = ioctl(handle, PPS_IOC_FETCH, &arg);
	*ppsinfobuf = arg.pps_info_buf;
	return (error);
}

/* specify kernel consumer */
static __inline int time_pps_kcbind(pps_handle_t handle,
				    const int kernel_consumer,
				    const int edge, const int tsformat)
{
	int error;
	struct pps_bind_args arg;

	arg.tsformat = tsformat;
	arg.edge = edge;
	arg.consumer = kernel_consumer;
	error = ioctl(handle, PPS_IOC_KC_BIND, &arg);
	return(error);
}

#endif /* !__KERNEL__ */
#endif /* _SYS_TIMEPPS_H_ */
