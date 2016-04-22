/***********************************************************************
 *								       *
 * Copyright (c) David L. Mills 1993-2000			       *
 *								       *
 * Permission to use, copy, modify, and distribute this software and   *
 * its documentation for any purpose and without fee is hereby	       *
 * granted, provided that the above copyright notice appears in all    *
 * copies and that both the copyright notice and this permission       *
 * notice appear in supporting documentation, and that the name        *
 * University of Delaware not be used in advertising or publicity      *
 * pertaining to distribution of the software without specific,	       *
 * written prior permission. The University of Delaware makes no       *
 * representations about the suitability this software for any	       *
 * purpose. It is provided "as is" without express or implied	       *
 * warranty.							       *
 *								       *
 **********************************************************************/

/*
 * Modification history timex.h
 *
 * 17 Nov 98	David L. Mills
 *	Revised for nanosecond kernel and user interface.
 * 29 Dec 97	Russell King
 *	Moved CLOCK_TICK_RATE, CLOCK_TICK_FACTOR and FINETUNE to asm/timex.h
 *	for ARM machines
 *
 *  9 Jan 97    Adrian Sun
 *      Shifted LATCH define to allow access to alpha machines.
 *
 * 26 Sep 94	David L. Mills
 *	Added defines for hybrid phase/frequency-lock loop.
 *
 * 19 Mar 94	David L. Mills
 *	Moved defines from kernel routines to header file and added new
 *	defines for PPS phase-lock loop.
 *
 * 20 Feb 94	David L. Mills
 *	Revised status codes and structures for external clock and PPS
 *	signal discipline.
 *
 * 28 Nov 93	David L. Mills
 *	Adjusted parameters to improve stability and increase poll
 *	interval.
 *
 * 17 Sep 93    David L. Mills
 *      Created file $NTP/include/sys/timex.h
 * 07 Oct 93    Torsten Duwe
 *      Derived linux/timex.h
 * 1995-08-13    Torsten Duwe
 *      kernel PLL updated to 1994-12-13 specs (rfc-1589)
 * 1997-08-30    Ulrich Windl
 *      Added new constant NTP_PHASE_LIMIT
 * 2004-08-12    Christoph Lameter
 *      Reworked time interpolation logic
 * 2006-02-ß7    Ulrich Windl
 *      Merge Dave Mills code base with Linux code base
 */
#ifndef _LINUX_TIMEX_H
#define _LINUX_TIMEX_H

#include <linux/compiler.h>
#include <linux/time.h>

#include <asm/param.h>

/*
 * This header file defines the Network Time Protocol (NTP) interfaces
 * for user and daemon application programs. These are implemented using
 * defined syscalls and data structures and require specific kernel
 * support.
 *
 * The original precision time kernels developed from 1993 have an
 * ultimate resolution of one microsecond; however, the most recent
 * kernels have an ultimate resolution of one nanosecond. In these
 * kernels, a ntp_adjtime() syscalls can be used to determine which
 * resolution is in use and to select either one at any time. The
 * resolution selected affects the scaling of certain fields in the
 * ntp_gettime() and ntp_adjtime() syscalls, as described below.
 *
 * NAME
 *	ntp_gettime - NTP user application interface
 *
 * SYNOPSIS
 *	#include <sys/timex.h>
 *
 *	int ntp_gettime(struct ntptimeval *tptr);
 *
 * DESCRIPTION
 *	The time returned by ntp_gettime() is in a timeval structure,
 *	but may be in either microsecond (seconds and microseconds) or
 *	nanosecond (seconds and nanoseconds) format. The particular
 *	format in use is determined by the STA_NANO bit of the status
 *	word returned by the ntp_adjtime() syscall.
 *
 * NAME
 *	ntp_adjtime - NTP daemon application interface
 *
 * SYNOPSIS
 *	#include <sys/timex.h>
 *
 *	int ntp_adjtime(struct timex *tptr);
 *
 * DESCRIPTION
 *	Certain fields of the timex structure are interpreted in either
 *	microseconds or nanoseconds according to the state of the
 *	STA_NANO bit in the status word. See the description below for
 *	further information.
 */
/*
 * syscall interface - used (mainly by NTP daemon)
 * to discipline kernel clock oscillator
 */
struct timex {
	unsigned int modes;	/* clock mode bits (wo) */
	long	offset;		/* time offset (ns/us) (rw) */
	long	freq;		/* frequency offset (scaled PPM) (rw) */
	long	maxerror;	/* maximum error (us) (rw) */
	long	esterror;	/* estimated error (us) (rw) */
	int	status;		/* clock status bits (rw) */
	long	constant;	/* poll interval (log2 s) (rw) */
	long	precision;	/* clock precision (ns/us) (ro) */
	long	tolerance;	/* clock frequency tolerance (scaled PPM) (ro)
				 */

	/*----BEGIN Linux extensions (part 1) */
	/* Depending on whether ``NTP_NANO'' is defined when including
	 * this file, the same memory location can be accessed under
	 * two different names.  ``STA_NANO'' at runtime should
	 * correspond with ``NTP_NANO'' at compilation time.
	 */
#ifdef NTP_NANO
	struct timespec time;	/* current time (ns) (ro) */
#else
	struct timeval time;	/* current time (us) (ro) */
#endif /* NTP_NANO */
	long	tick;		/* (modified) usecs between clock ticks (rw) */
	/*----END Linux extensions (part 1) */

	long ppsfreq;           /* PPS frequency (scaled PPM) (ro) */
	long jitter;            /* PPS jitter (ns/us) (ro) */
	int shift;              /* interval duration (s) (shift) (ro) */
	long stabil;            /* PPS stability (scaled PPM) (ro) */
	long jitcnt;            /* jitter limit exceeded (ro) */
	long calcnt;            /* calibration intervals (ro) */
	long errcnt;            /* calibration errors (ro) */
	long stbcnt;            /* stability limit exceeded (ro) */

	/*----BEGIN Linux extensions (part 2) */
	/* This is a fixed block with some reserved space (12 * sizeof(int))
	 * initially. Now we add new stuff, replacing those placeholders
	 * unused so far. Yes, it's ugly, but it's binary compatible
	 * (where sizeof(int) == sizeof(long)).
	 */
	long	tai;		/* TAI offset */
	int  :32; int  :32; int  :32;
	int	tickadj;	/* tickadj (us) (rw) -- extension by UW */
	int  :32; int  :32; int  :32;
	int  :32; int  :32; int  :32; int  :32;
	/*----END Linux extensions (part 2) */
};

#define	NTP_API		4	/* NTP API version */
/*
 * The following defines establish the performance envelope of the
 * kernel discipline loop. Phase or frequency errors greater than
 * NAXPHASE or MAXFREQ are clamped to these maxima. For update intervals
 * less than MINSEC, the loop always operates in PLL mode; while, for
 * update intervals greater than MAXSEC, the loop always operates in FLL
 * mode. Between these two limits the operating mode is selected by the
 * STA_FLL bit in the status word.
 */
#define MAXPHASE	500000000L /* max phase error (ns) */
#define MAXFREQ		500000L	/* max freq error (ns/s) */
#define MINSEC		256	/* min FLL update interval (s) */
#define MAXSEC		2048	/* max PLL update interval (s) */
#define NANOSECOND	1000000000L /* nanoseconds in one second */
#define SCALE_PPM	(65536 / 1000) /* crude ns/s to scaled PPM */
#define MAXTC		10	/* max time constant */

/*
 * The following defines and structures define the user interface for
 * the ntp_gettime() and ntp_adjtime() syscalls.
 *
 * Control mode codes (timex.modes)
 */
#define MOD_OFFSET	0x0001	/* set time offset */
#define MOD_FREQUENCY	0x0002	/* set frequency offset */
#define MOD_MAXERROR	0x0004	/* set maximum time error */
#define MOD_ESTERROR	0x0008	/* set estimated time error */
#define MOD_STATUS	0x0010	/* set clock status bits */
#define MOD_TIMECONST	0x0020	/* set PLL time constant */
#define MOD_PPSMAX	0x0040	/* set PPS maximum averaging interval */
#define MOD_TAI		0x0080	/* set TAI offset */
#define	MOD_MICRO	0x1000	/* select microsecond resolution */
#define	MOD_NANO	0x2000	/* select nanosecond resolution */
#define MOD_CLKB	0x4000	/* select clock B */
#define MOD_CLKA	0x8000	/* select clock A */
  
/*
 * Linux extensions:
 * Here are the additional bits used to add adjtime() and other related
 * functionality.  They all start with ``ADJ_''.  Some of these bits were
 * different in the older kernel's adjtimex(), thus not all old binaries
 * will work!!! (However those binaries are all system-level)
 * We've learned the lesson and allocate bits from the other end now!
 * As the `ADJ_OFFSET_SINGLESHOT' is the candidate that will most likely
 * cause trouble, take the chance and replace that ugly name with
 * `ADJ_ADJTIME' (because it's doing adjtime() anyway).
 */
#define ADJ_ADJTIME	0x80000000	/* plain old adjtime() */
#define ADJ_TIMETICK	0x40000000	/* set new value for  `time_tick' */
#define ADJ_TICKADJ	0x20000000	/* set new value for `tickadj' */

/*FIXME: these should vanish */
#define ADJ_TICK	_use_ADJ_TIMETICK_instead_	/*FIXME:0x4000 == tick value */
#define ADJ_OFFSET_SINGLESHOT	_use_ADJ_ADJTIME_instead_	/*FIXME:0x8001 == old-fashioned adjtime */

/*
 * Status codes (timex.status)
 */
#define STA_PLL		0x0001	/* enable PLL updates (rw) */
#define STA_PPSFREQ	0x0002	/* enable PPS freq discipline (rw) */
#define STA_PPSTIME	0x0004	/* enable PPS time discipline (rw) */
#define STA_FLL		0x0008	/* enable FLL mode (rw) */

#define STA_INS		0x0010	/* insert leap (rw) */
#define STA_DEL		0x0020	/* delete leap (rw) */
#define STA_UNSYNC	0x0040	/* clock unsynchronized (rw) */
#define STA_FREQHOLD	0x0080	/* hold frequency (rw) */

#define STA_PPSSIGNAL	0x0100	/* PPS signal present (ro) */
#define STA_PPSJITTER	0x0200	/* PPS signal jitter exceeded (ro) */
#define STA_PPSWANDER	0x0400	/* PPS signal wander exceeded (ro) */
#define STA_PPSERROR	0x0800	/* PPS signal calibration error (ro) */

#define STA_CLOCKERR	0x1000	/* clock hardware fault (ro) */
#define STA_NANO	0x2000	/* resolution (0 = us, 1 = ns) (ro) */
#define STA_MODE	0x4000	/* mode (0 = PLL, 1 = FLL) (ro) */
#define STA_CLK		0x8000	/* clock source (0 = A, 1 = B) (ro) */

/* read-only bits */
#define STA_RONLY (STA_PPSSIGNAL | STA_PPSJITTER | STA_PPSWANDER | \
    STA_PPSERROR | STA_CLOCKERR | STA_NANO | STA_MODE | STA_CLK)

/*
 * Clock states (time_state)
 */
#define TIME_OK		0	/* no leap second warning */
#define TIME_INS	1	/* insert leap second warning */
#define TIME_DEL	2	/* delete leap second warning */
#define TIME_OOP	3	/* leap second in progress */
#define TIME_WAIT	4	/* leap second has occurred */
#define TIME_ERROR	5	/* error (see status word) */

/*
 * NTP user interface (ntp_gettime()) - used to read kernel clock values
 *
 * Note: The time member is in microseconds if STA_NANO is zero and
 * nanoseconds if not.  Depending on whether ``NANO'' is defined when
 * including this file, the same memory location can be accessed under two
 * different names.  ``STA_NANO'' at runtime should correspond with ``NANO''
 * at compilation time.
 */
struct ntptimeval {
#ifdef NTP_NANO
	struct timespec time;	/* current time (ns) (ro) */
#else
	struct timeval time;	/* current time (us) (ro) */
#endif /* NTP_NANO */
	long maxerror;		/* maximum error (us) (ro) */
	long esterror;		/* estimated error (us) (ro) */
	long tai;		/* TAI offset */
};

#ifdef __KERNEL__
#include <asm/timex.h>

/*
 * kernel variables
 * Note: maximum error = NTP synch distance = dispersion + delay / 2;
 * estimated error = NTP dispersion.
 */
extern unsigned long tick_usec;		/* USER_HZ period (usec) */
extern unsigned long tick_nsec;		/* ACTHZ   period (nsec) */

/* This limit (ns) will set the clock to `unsynchronized' */
#define	NTP_PHASE_LIMIT	(MAXPHASE << 2)
  
extern void set_ntp_unsync(void);	/* tell NTP the clock was set */
#define ntp_clear	set_ntp_unsync
extern int ntp_synced(void);		/* is NTP clock in sync? */

/* Required to safely shift negative values */
#define shift_right(x, s) ({	\
	__typeof__(x) __x = (x);	\
	__typeof__(s) __s = (s);	\
	__x < 0 ? -(-__x >> __s) : __x >> __s;	\
})


#ifdef CONFIG_TIME_INTERPOLATION

#define TIME_SOURCE_CPU 0
#define TIME_SOURCE_MMIO64 1
#define TIME_SOURCE_MMIO32 2
#define TIME_SOURCE_FUNCTION 3

/* For proper operations time_interpolator clocks must run slightly slower
 * than the standard clock since the interpolator may only correct by having
 * time jump forward during a tick. A slower clock is usually a side effect
 * of the integer divide of the nanoseconds in a second by the frequency.
 * The accuracy of the division can be increased by specifying a shift.
 * However, this may cause the clock not to be slow enough.
 * The interpolator will self-tune the clock by slowing down if no
 * resets occur or speeding up if the time jumps per analysis cycle
 * become too high.
 *
 * Setting jitter compensates for a fluctuating timesource by comparing
 * to the last value read from the timesource to insure that an earlier value
 * is not returned by a later call. The price to pay
 * for the compensation is that the timer routines are not as scalable anymore.
 */

struct time_interpolator {
	u16 source;			/* time source flags */
	u8 shift;			/* increases accuracy of multiply by shifting. */
				/* Note that bits may be lost if shift is set too high */
	u8 jitter;			/* if set compensate for fluctuations */
	u32 nsec_per_cyc;		/* set by register_time_interpolator() */
	void *addr;			/* address of counter or function */
	u64 mask;			/* mask the valid bits of the counter */
	unsigned long offset;		/* nsec offset at last update of interpolator */
	u64 last_counter;		/* counter value in units of the counter at last update */
	u64 last_cycle;			/* Last timer value if TIME_SOURCE_JITTER is set */
	u64 frequency;			/* frequency in counts/second */
	long drift;			/* drift in parts-per-million (or -1) */
	unsigned long skips;		/* skips forward */
	unsigned long ns_skipped;	/* nanoseconds skipped */
	struct time_interpolator *next;
};

extern void register_time_interpolator(struct time_interpolator *);
extern void unregister_time_interpolator(struct time_interpolator *);
extern void time_interpolator_reset(void);
extern unsigned long time_interpolator_get_offset(void);

#else /* !CONFIG_TIME_INTERPOLATION */

static inline void
time_interpolator_reset(void)
{
}

#endif /* !CONFIG_TIME_INTERPOLATION */

#define TICK_LENGTH_SHIFT	32

/* Returns how long ticks are at present, in ns / 2^(SHIFT_SCALE-10). */
extern u64 current_tick_length(void);

extern int do_adjtimex(struct timex *);

#endif /* KERNEL */

#endif /* LINUX_TIMEX_H */
