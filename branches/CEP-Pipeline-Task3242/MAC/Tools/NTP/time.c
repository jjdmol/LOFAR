/*
 *  linux/kernel/time.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  This file contains the interface functions for the various
 *  time related system calls: time, stime, gettimeofday, settimeofday,
 *			       adjtime
 */
/*
 * Modification history kernel/time.c
 * 
 * 1993-09-02    Philip Gladstone
 *      Created file with time related functions from sched.c and adjtimex() 
 * 1993-10-08    Torsten Duwe
 *      adjtime interface update and CMOS clock write code
 * 1995-08-13    Torsten Duwe
 *      kernel PLL updated to 1994-12-13 specs (rfc-1589)
 * 1999-01-16    Ulrich Windl
 *	Introduced error checking for many cases in adjtimex().
 *	Updated NTP code according to technical memorandum Jan '96
 *	"A Kernel Model for Precision Timekeeping" by Dave Mills
 *	Allow time_constant larger than MAXTC(6) for NTP v4 (MAXTC == 10)
 *	(Even though the technical memorandum forbids it)
 * 2004-07-14	 Christoph Lameter
 *	Added getnstimeofday to allow the posix timer functions to return
 *	with nanosecond accuracy
 * 2006-08-12	Ulrich Windl
 *	Merge nanokernel implementation for Linux 2.4 with current code.
 *	Fix ``getnstimeofday()'' to not discard the nanoseconds. Reduce
 *	code duplication.
 */
#define	NTP_NANO

#include <linux/module.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/smp_lock.h>
#include <linux/syscalls.h>
#include <linux/security.h>
#include <linux/fs.h>
#include <linux/module.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>

/* 
 * The timezone where the local system is located.  Used as a default by some
 * programs who obtain this value by using gettimeofday.
 */
struct timezone sys_tz;

EXPORT_SYMBOL(sys_tz);

unsigned long tick_usec = TICK_USEC; 		/* USER_HZ period (usec) */
unsigned long tick_nsec = TICK_NSEC;		/* ACTHZ period (nsec) */

/* 
 * The current time 
 * wall_to_monotonic is what we need to add to xtime (or xtime corrected 
 * for sub jiffie times) to get to monotonic time.  Monotonic is pegged
 * at zero at system boot time, so wall_to_monotonic will be negative,
 * however, we will ALWAYS keep the tv_nsec part positive so we can use
 * the usual normalization.
 */
struct timespec xtime __attribute__ ((aligned (16)));
struct timespec wall_to_monotonic __attribute__ ((aligned (16)));

EXPORT_SYMBOL(xtime);

/* kernel time variables */
#ifdef REO
static long time_adjust = 0;	/* remaining adjustment for adjtime() */
static int tickadj;		/* number of microseconds to adjust tick_nsec */
#else
// These values are now also needed in timer.c
long time_adjust = 0;	/* remaining adjustment for adjtime() */
int tickadj;		/* number of microseconds to adjust tick_nsec */
#endif

/* [extension by UW]: If ``rtc_update'' is positive then update the RTC as
 * before, but using the value as interval between updates.
 * ``rtc_update_slave'' will contain the value of ``rtc_update'' if the RTC
 * clock should be updated.  Once updated, the value will be reset to zero.
 */
static	int rtc_update;
int rtc_update_slave;

/* [extension by UW]: If ``rtc_runs_localtime'' is non-zero, believe
 * that the RTC uses local time.
 */
static	int rtc_runs_localtime = 0;

#define hz HZ			/* (fixed) timer interrupt frequency */

#define	ADJ_SERVED_MODE_BITS	(MOD_OFFSET|MOD_FREQUENCY|MOD_MAXERROR| \
				 MOD_ESTERROR|MOD_STATUS|MOD_TIMECONST| \
				 MOD_PPSMAX|MOD_TAI|MOD_MICRO|MOD_NANO| \
				 MOD_CLKB|MOD_CLKA| \
				 ADJ_TICKADJ|ADJ_TIMETICK|ADJ_ADJTIME)
/*
 * Generic NTP kernel interface
 *
 * These routines constitute the Network Time Protocol (NTP) interfaces
 * for user and daemon application programs. The ntp_gettime() routine
 * provides the time, maximum error (synch distance) and estimated error
 * (dispersion) to client user application programs. The ntp_adjtime()
 * routine is used by the NTP daemon to adjust the system clock to an
 * externally derived time. The time offset and related variables set by
 * this routine are used by other routines in this module to adjust the
 * phase and frequency of the clock discipline loop which controls the
 * system clock.
 *
 * When the kernel time is reckoned directly in nanoseconds (NTP_NANO
 * defined), the time at each tick interrupt is derived directly from
 * the kernel time variable. When the kernel time is reckoned in
 * microseconds, (NTP_NANO undefined), the time is derived from the kernel
 * time variable together with a variable representing the leftover
 * nanoseconds at the last tick interrupt. In either case, the current
 * nanosecond time is reckoned from these values plus an interpolated
 * value derived by the clock routines in another architecture-specific
 * module. The interpolation can use either a dedicated counter or a
 * processor cycle counter (PCC) implemented in some architectures.
 *
 * Note that all routines must run with exclusive access to the time
 * variables.
 */
#include <linux/timex.h>
#include <linux/l_fp.h>
/*
 * Phase/frequency-lock loop (PLL/FLL) definitions
 *
 * The nanosecond clock discipline uses two variable types, time
 * variables and frequency variables. Both types are represented as 64-
 * bit fixed-point quantities with the decimal point between two 32-bit
 * halves. On a 32-bit machine, each half is represented as a single
 * word and mathematical operations are done using multiple-precision
 * arithmetic. On a 64-bit machine, ordinary computer arithmetic is
 * used.
 *
 * A time variable is a signed 64-bit fixed-point number in ns and
 * fraction. It represents the remaining time offset to be amortized
 * over succeeding tick interrupts. The maximum time offset is about
 * 0.5 s and the resolution is about 2.3e-10 ns.
 *
 *			1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |s s s|			 ns				   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |			    fraction				   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * A frequency variable is a signed 64-bit fixed-point number in ns/s
 * and fraction. It represents the ns and fraction to be added to the
 * kernel time variable at each second. The maximum frequency offset is
 * about +-500000 ns/s and the resolution is about 2.3e-10 ns/s.
 *
 *			1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |s s s s s s s s s s s s s|	          ns/s			   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |			    fraction				   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
/*
 * The following variables establish the state of the PLL/FLL and the
 * residual time and frequency offset of the local clock.
 */
#define SHIFT_PLL	4	/* PLL loop gain (shift) */
#define SHIFT_FLL	2	/* FLL loop gain (shift) */

int time_state;			/* clock state */
/* bit STA_UNSYNC prevents a periodic update of the CMOS clock */
int time_status;		/* clock status bits */
long time_constant;		/* poll interval (shift) (s) */
long time_tai;			/* TAI offset (s) */
long time_monitor;		/* last time offset scaled (ns) */
long time_precision = 1;	/* clock precision (ns) */
long time_maxerror;		/* maximum error (ns) -- [extension by UW] */
long time_esterror;		/* estimated error (ns) -- [extension by UW] */
long time_reftime;		/* time at last adjustment (s) */
l_fp time_offset;		/* time offset (ns) */
l_fp time_freq;			/* frequency offset (ns/s) */
l_fp time_adj;			/* tick adjust (ns/s) */
l_fp time_phase;		/* time phase (ns) */

#ifdef CONFIG_NTP_PPS
/*
 * The following variables are used when a pulse-per-second (PPS) signal
 * is available and connected via a modem control lead. They establish
 * the engineering parameters of the clock discipline loop when
 * controlled by the PPS signal.
 */
#define PPS_FAVG	2	/* min freq avg interval (s) (shift) */
#define PPS_FAVGDEF	8	/* default freq avg interval (s) (shift) */
#define PPS_FAVGMAX	15	/* max freq avg interval (s) (shift) */
#define PPS_PAVG	4	/* phase avg interval (s) (shift) */
#define PPS_VALID	120	/* PPS signal watchdog max (s) */
#define PPS_MAXWANDER	100000	/* max PPS wander (ns/s) */
#define PPS_POPCORN	2	/* popcorn spike threshold (shift) */

struct pps_var {
	struct timespec tf[3];	/* phase median filter */
	l_fp freq;		/* scaled frequency offset (ns/s) */
	long lastfreq;		/* last scaled freq offset (ns/s) */
	long fcount;		/* frequency accumulator */
	long jitter;		/* nominal jitter (ns) */
	long stabil;		/* nominal stability (scaled ns/s) */
	long lastsec;		/* time of last calibration (s) */
	int valid;		/* signal watchdog counter */
	int shift;		/* interval duration (s) (shift) */
	int shiftmax;		/* max interval duration (s) (shift) */
	int intcnt;		/* interval counter */

/*
 * PPS signal quality monitors
 */
	long calcnt;		/* calibration intervals */
	long jitcnt;		/* jitter limit exceeded */
	long stbcnt;		/* stability limit exceeded */
	long errcnt;		/* calibration errors */

/* dynamic tolerance adjustment */
	long fmin, fmax;	/* PPS frequency minimum and maximum */
	long frange;		/* frequency range */
};

static struct pps_var pps;
#endif	/* CONFIG_NTP_PPS */

/**
 * set_ntp_unsync - set NTP clock to unsynchronized (e.g. after setting time)
 *
 * Must be called while holding a write on the xtime_lock
 */
void set_ntp_unsync(void)
{
	time_adjust = 0;		/* stop active adjtime() */
	time_status |= STA_UNSYNC;
	time_maxerror = NTP_PHASE_LIMIT;
	time_esterror = NTP_PHASE_LIMIT;
}

/* is NTP clock in sync? */
int ntp_synced(void)
{
	return (time_status & STA_UNSYNC) == 0;
}

#ifdef __ARCH_WANT_SYS_TIME

/*
 * sys_time() can be implemented in user-level using
 * sys_gettimeofday().  Is this for backwards compatibility?  If so,
 * why not move it into the appropriate arch directory (for those
 * architectures that need it).
 */
asmlinkage long sys_time(time_t __user * tloc)
{
	time_t i;
	struct timeval tv;

	do_gettimeofday(&tv);
	i = tv.tv_sec;

	if (tloc) {
		if (put_user(i,tloc))
			i = -EFAULT;
	}
	return i;
}

/*
 * this routine handles the overflow of the microsecond field
 *
 * The tricky bits of code to handle the accurate clock support
 * were provided by Dave Mills (Mills@UDEL.EDU) of NTP fame.
 * They were originally developed for SUN and DEC kernels.
 * All the kudos should go to Dave for this stuff.
 *
 */
void second_overflow(void)
{
	l_fp ftemp;		/* 32/64-bit temporary */

	/* Bump the maxerror field */
#ifdef CONFIG_NTP_PPS
	time_maxerror += pps.frange;
#else
	time_maxerror += MAXFREQ;
#endif
	if (time_maxerror > NTP_PHASE_LIMIT) {
		time_maxerror = NTP_PHASE_LIMIT;
		time_status |= STA_UNSYNC;
	}

	if (rtc_update > 0 && (time_status & STA_UNSYNC) == 0)
		rtc_update_slave = rtc_update;
	/*
	 * Leap second processing. If in leap-insert state at the end of the
	 * day, the system clock is set back one second; if in leap-delete
	 * state, the system clock is set ahead one second. The microtime()
	 * routine or external clock driver will insure that reported time is
	 * always monotonic. The ugly divides should be replaced.
	 */
	switch (time_state) {
	case TIME_OK:	/* no leap second */
		if (time_status & STA_INS)
			time_state = TIME_INS;
		else if (time_status & STA_DEL)
			time_state = TIME_DEL;
		break;
	case TIME_INS:	/* Insert second 23:59:60 following second 23:59:59. */
		if (!(time_status & STA_INS))
			time_state = TIME_OK;
		else if (xtime.tv_sec % 86400 == 0) {
			xtime.tv_sec--;
			wall_to_monotonic.tv_sec++;
			/*FIXME: what's that? (UW) */
			/*
			 * The timer interpolator will make time change
			 * gradually instead of an immediate jump by one second
			 */
			time_interpolator_update(-NSEC_PER_SEC);
			time_state = TIME_OOP;
			clock_was_set();
			printk(KERN_NOTICE
			       "TIME_INS: inserting second 23:59:60 UTC\n");
		}
		break;
	case TIME_DEL:	/* Skip second 23:59:59. */
		if (!(time_status & STA_DEL))
			time_state = TIME_OK;
		else if ((xtime.tv_sec + 1) % 86400 == 0) {
			xtime.tv_sec++;
			wall_to_monotonic.tv_sec--;
			time_tai--;
			/*
			 * Use of time interpolator for a gradual change of
			 * time
			 */
			time_interpolator_update(NSEC_PER_SEC);
			clock_was_set();
			time_state = TIME_WAIT;
			printk(KERN_NOTICE
			       "TIME_DEL: skipping second 23:59:59 UTC\n");
		}
		if ((xtime.tv_sec + 1) % 86400 == 0) {
			xtime.tv_sec++;
			wall_to_monotonic.tv_sec--;
			/*
			 * Use of time interpolator for a gradual change of
			 * time
			 */
			time_interpolator_update(NSEC_PER_SEC);
			time_state = TIME_WAIT;
			clock_was_set();
			printk(KERN_NOTICE "Clock: deleting leap second "
					"23:59:59 UTC\n");
		}
		break;
	case TIME_OOP:	/* leap second insert in progress */
		time_tai++;
		time_state = TIME_WAIT;
		break;
	case TIME_WAIT:	/* Wait for status bits to clear. */
		if (!(time_status & (STA_INS | STA_DEL)))
			time_state = TIME_OK;
	}

	/*
	 * Compute the total time adjustment for the next second in ns. The
	 * offset is reduced by a factor depending on whether the PPS signal
	 * is operating. Note that the value is in effect scaled by the clock
	 * frequency, since the adjustment is added at each tick interrupt.
	 */
	ftemp = time_offset;
#ifdef CONFIG_NTP_PPS
	if (time_status & STA_PPSTIME && time_status & STA_PPSSIGNAL)
		L_RSHIFT(ftemp, pps.shift);
	else
#endif
		L_RSHIFT(ftemp, SHIFT_PLL + time_constant);
	time_adj = ftemp;
	L_SUB(time_offset, ftemp);
	L_ADD(time_adj, time_freq);
	L_ADDHI(time_adj, NSEC_PER_SEC);
#ifdef	CONFIG_NTP_PPS
	if (pps.valid > 0)
		pps.valid--;
	else
		time_status &= ~(STA_PPSSIGNAL | STA_PPSJITTER |
				 STA_PPSWANDER | STA_PPSERROR);
#endif
#if 0 /*FIXME: What about this? (UW)*/
#if HZ == 100
	/*
	 * Compensate for (HZ==100) != (1 << SHIFT_HZ).  Add 25% and 3.125% to
	 * get 128.125; => only 0.125% error (p. 14)
	 */
	time_adj += shift_right(time_adj, 2) + shift_right(time_adj, 5);
#endif
#if HZ == 250
	/*
	 * Compensate for (HZ==250) != (1 << SHIFT_HZ).  Add 1.5625% and
	 * 0.78125% to get 255.85938; => only 0.05% error (p. 14)
	 */
	time_adj += shift_right(time_adj, 6) + shift_right(time_adj, 7);
#endif
#if HZ == 1000
	/*
	 * Compensate for (HZ==1000) != (1 << SHIFT_HZ).  Add 1.5625% and
	 * 0.78125% to get 1023.4375; => only 0.05% error (p. 14)
	 */
	time_adj += shift_right(time_adj, 6) + shift_right(time_adj, 7);
#endif
#endif
}

/* in the NTP reference this is called "hardclock()" */
void update_wall_time_one_tick(void)
{
	long ltemp, time_update = time_adjust;

	if (unlikely(time_update != 0)) {	/* doing adjtime() */
		/* We are doing an adjtime() thing.
		 * Clamp time_update within bounds (-tickadj .. tickadj).
		 */
		if (time_update > tickadj)
			time_update = tickadj;
		else if (time_update < -tickadj)
			time_update = -tickadj;
	     
		/* Update remaining adjustment */
		time_adjust -= time_update;
		time_update *= 1000;	/* convert microseconds to nanoseconds */
	}
	/*
	 * Update the nanosecond and microsecond clocks. If the phase
	 * increment exceeds the tick period, update the clock phase.
	 */
	L_ADD(time_phase, time_adj);
	ltemp = L_GINT(time_phase) / hz;
	time_update += ltemp;
	L_ADDHI(time_phase, -ltemp * hz);
	xtime.tv_nsec += time_update;
	time_interpolator_update(time_update);
}

/*
 * sys_stime() can be implemented in user-level using
 * sys_settimeofday().  Is this for backwards compatibility?  If so,
 * why not move it into the appropriate arch directory (for those
 * architectures that need it).
 */
 
asmlinkage long sys_stime(time_t __user *tptr)
{
	struct timespec tv;
	int err;

	if (get_user(tv.tv_sec, tptr))
		return -EFAULT;

	tv.tv_nsec = 0;

	err = security_settime(&tv, NULL);
	if (err)
		return err;

	do_settimeofday(&tv);
	return 0;
}

#endif /* __ARCH_WANT_SYS_TIME */

asmlinkage long sys_gettimeofday(struct timeval __user *tv, struct timezone __user *tz)
{
	if (likely(tv != NULL)) {
		struct timeval ktv;
		do_gettimeofday(&ktv);
		if (copy_to_user(tv, &ktv, sizeof(ktv)))
			return -EFAULT;
	}
	if (unlikely(tz != NULL)) {
		if (copy_to_user(tz, &sys_tz, sizeof(sys_tz)))
			return -EFAULT;
	}
	return 0;
}

/*
 * Adjust the time obtained from the CMOS to be UTC time instead of
 * local time.
 * 
 * This is ugly, but preferable to the alternatives.  Otherwise we
 * would either need to write a program to do it in /etc/rc (and risk
 * confusion if the program gets run more than once; it would also be 
 * hard to make the program warp the clock precisely n hours)  or
 * compile in the timezone information into the kernel.  Bad, bad....
 *
 *              				- TYT, 1992-01-01
 *
 * The best thing to do is to keep the CMOS clock in universal time (UTC)
 * as real UNIX machines always do it. This avoids all headaches about
 * daylight saving times and warping kernel clocks.
 */
static inline void warp_clock(void)
{
	write_seqlock_irq(&xtime_lock);
	wall_to_monotonic.tv_sec -= sys_tz.tz_minuteswest * 60;
	xtime.tv_sec += sys_tz.tz_minuteswest * 60;
	time_interpolator_reset();
	write_sequnlock_irq(&xtime_lock);
	clock_was_set();
}

/*
 * In case for some reason the CMOS clock has not already been running
 * in UTC, but in some local time: The first time we set the timezone,
 * we will warp the clock so that it is ticking UTC time instead of
 * local time. Presumably, if someone is setting the timezone then we
 * are running in an environment where the programs understand about
 * timezones. This should be done at boot time in the /etc/rc script,
 * as soon as possible, so that the clock can be set right. Otherwise,
 * various programs will get confused when the clock gets warped.
 */

int do_sys_settimeofday(struct timespec *tv, struct timezone *tz)
{
	static int firsttime = 1;
	int error = 0;

	if (tv && !timespec_valid(tv))
		return -EINVAL;

	error = security_settime(tv, tz);
	if (error)
		return error;

	if (tz) {
		/* SMP safe, global irq locking makes it work. */
		sys_tz = *tz;
		if (firsttime) {
			firsttime = 0;
			if (!tv)
				warp_clock();
		}
	}
	if (tv)
	{
		/* SMP safe, again the code in arch/foo/time.c should
		 * globally block out interrupts when it runs.
		 */
		return do_settimeofday(tv);
	}
	return 0;
}

asmlinkage long sys_settimeofday(struct timeval __user *tv,
				struct timezone __user *tz)
{
	struct timeval user_tv;
	struct timespec	new_ts;
	struct timezone new_tz;

	if (tv) {
		if (copy_from_user(&user_tv, tv, sizeof(*tv)))
			return -EFAULT;
		new_ts.tv_sec = user_tv.tv_sec;
		new_ts.tv_nsec = user_tv.tv_usec * NSEC_PER_USEC;
	}
	if (tz) {
		if (copy_from_user(&new_tz, tz, sizeof(*tz)))
			return -EFAULT;
	}

	return do_sys_settimeofday(tv ? &new_ts : NULL, tz ? &new_tz : NULL);
}

/*
 * The following variables are used when a pulse-per-second (PPS) signal
 * is available and connected via a modem control lead. They establish
 * the engineering parameters of the clock discipline loop when
 * controlled by the PPS signal.
 */
#define PPS_FAVG	2	/* min freq avg interval (s) (shift) */
#define PPS_FAVGDEF	8	/* default freq avg interval (s) (shift) */
#define PPS_FAVGMAX	15	/* max freq avg interval (s) (shift) */
#define PPS_PAVG	4	/* phase avg interval (s) (shift) */
#define PPS_VALID	120	/* PPS signal watchdog max (s) */
#define PPS_MAXWANDER	100000	/* max PPS wander (ns/s) */
#define PPS_POPCORN	2	/* popcorn spike threshold (shift) */

long pps_offset;		/* pps time offset (us) */
long pps_jitter;		/* time dispersion (jitter) (us) */

long pps_freq;			/* frequency offset (scaled ppm) */
long pps_stabil;		/* frequency dispersion (scaled ppm) */

long pps_valid;			/* pps signal watchdog counter */

int pps_shift = PPS_FAVG;	/* interval duration (s) (shift) */

long pps_jitcnt;		/* jitter limit exceeded */
long pps_calcnt;		/* calibration intervals */
long pps_errcnt;		/* calibration errors */
long pps_stbcnt;		/* stability limit exceeded */

/* we call this to notify the arch when the clock is being
 * controlled.  If no such arch routine, do nothing.
 */
void __attribute__ ((weak)) notify_arch_cmos_timer(void)
{
	return;
}

#ifdef	CONFIG_NTP_PPS
/* this is an ugly work-around implementation for the overly
 * complicated existing POSIX routines
 */
unsigned long do_clock_gettime(int unused_dummy, struct timespec *ts)
{
	getnstimeofday(ts);
	return ts->tv_nsec % tick_nsec;
}
EXPORT_SYMBOL(do_clock_gettime);

/*
 * hardpps() - discipline CPU clock oscillator to external PPS signal
 *
 * This routine is called at each PPS interrupt in order to discipline
 * the CPU clock oscillator to the PPS signal. There are two independent
 * first-order feedback loops, one for the phase, the other for the
 * frequency. The phase loop measures and grooms the PPS phase offset
 * and leaves it in a handy spot for the seconds overflow routine. The
 * frequency loop averages successive PPS phase differences and
 * calculates the PPS frequency offset, which is also processed by the
 * seconds overflow routine. The code requires the caller to capture the
 * time and architecture-dependent hardware counter values in
 * nanoseconds at the on-time PPS signal transition.
 *
 * Note that, on some Unix systems this routine runs at an interrupt
 * priority level higher than the timer interrupt routine second_overflow().
 * Therefore, the variables used are distinct from the second_overflow()
 * variables, except for the actual time and frequency variables, which
 * are determined by this routine and updated atomically.
 */
void hardpps(const struct timespec *p_ts, long nsec)
{
	static	long pps_lastcount;	/* last counter offset */
	unsigned long flags;
	long u_sec, u_nsec, v_nsec;	/* multi-purpose temps */
	l_fp ftemp;
	int fr_upd;			/* update frequency range? */

	write_seqlock_irqsave(&xtime_lock, flags);
	/*
	 * The signal is first processed by a range gate and frequency
	 * discriminator. The range gate rejects noise spikes outside
	 * the range +-500 us. The frequency discriminator rejects input
	 * signals with apparent frequency outside the range 1 +-500
	 * PPM. If two hits occur in the same second, we ignore the
	 * later hit; if not and a hit occurs outside the range gate,
	 * keep the later hit for later comparison, but do not process
	 * it.
	 */
	time_status |= STA_PPSJITTER;
	time_status &= ~(STA_PPSWANDER | STA_PPSERROR);
	u_sec = p_ts->tv_sec;
	u_nsec = p_ts->tv_nsec;		/* relative position around second */
	if (u_nsec >= (NSEC_PER_SEC >> 1)) {
		u_nsec -= NSEC_PER_SEC;
		u_sec++;
	}
	/* compute relative offset to last position */
	v_nsec = u_nsec - pps.tf[0].tv_nsec;
	if (unlikely(u_sec == pps.tf[0].tv_sec &&
		     v_nsec < NSEC_PER_SEC - MAXFREQ)) {
#ifdef CONFIG_NTP_PPS_DEBUG
		printk(KERN_INFO
		       "hardpps(%ld.%08ld): early pulse, delta = %ld ns\n",
		       p_ts->tv_sec % 10, p_ts->tv_nsec, v_nsec);
#endif
		goto done;			/* ignore early pulse */
	}
	pps.tf[2] = pps.tf[1];
	pps.tf[1] = pps.tf[0];
	pps.tf[0].tv_sec = u_sec;
	pps.tf[0].tv_nsec = u_nsec;

	/*
	 * Compute the difference between the current and previous
	 * counter values. If the difference exceeds 0.5 s, assume it
	 * has wrapped around, so correct 1.0 s. If the result exceeds
	 * the tick interval, the sample point has crossed a tick
	 * boundary during the last second, so correct the tick. Very
	 * intricate.
	 */
	u_nsec = nsec - pps_lastcount;
	pps_lastcount = nsec;
	if (u_nsec > (tick_nsec >> 1))
		u_nsec -= tick_nsec;
	else if (u_nsec < -(tick_nsec >> 1))
		u_nsec += tick_nsec;
	pps.fcount += u_nsec;
	if (unlikely(v_nsec > MAXFREQ || v_nsec < -MAXFREQ)) {
#ifdef CONFIG_NTP_PPS_DEBUG
		printk(KERN_INFO "hardpps: bad pulse, delta = %ld ns\n",
		       v_nsec);
		printk(KERN_DEBUG
		       "hardpps: last=%ld:%09ld, this=%ld:%09ld (%ld nsec)\n",
		       pps.tf[1].tv_sec % 10, pps.tf[1].tv_nsec,
		       pps.tf[0].tv_sec % 10, pps.tf[0].tv_nsec, u_nsec);
#endif
		/* Not in reference implementation: The pulse is already in
		 * the filter stages, therefore we can't simply ignore it.
		 * Instead we discard the current data and restart the
		 * calibration interval.  Don't increment jitcnt.
		 */
		pps.lastsec = pps.tf[0].tv_sec;
		pps.fcount = 0;
		if (--pps.intcnt <= -4) {
			pps.intcnt = -4;
			if (pps.shift > PPS_FAVG) {
				pps.shift--;
				pps.intcnt = 0;
			}
		}
		goto done;
	}
	pps.valid = PPS_VALID;
	time_status |= STA_PPSSIGNAL;
	time_status &= ~STA_PPSJITTER;

	/*
	 * A three-stage median filter is used to help denoise the PPS
	 * time. The median sample becomes the time offset estimate; the
	 * difference between the other two samples becomes the time
	 * dispersion (jitter) estimate.
	 */
	if (pps.tf[0].tv_nsec > pps.tf[1].tv_nsec) {
		if (pps.tf[1].tv_nsec > pps.tf[2].tv_nsec) {
			v_nsec = pps.tf[1].tv_nsec;	/* 0 1 2 */
			u_nsec = pps.tf[0].tv_nsec - pps.tf[2].tv_nsec;
		} else if (pps.tf[2].tv_nsec > pps.tf[0].tv_nsec) {
			v_nsec = pps.tf[0].tv_nsec;	/* 2 0 1 */
			u_nsec = pps.tf[2].tv_nsec - pps.tf[1].tv_nsec;
		} else {
			v_nsec = pps.tf[2].tv_nsec;	/* 0 2 1 */
			u_nsec = pps.tf[0].tv_nsec - pps.tf[1].tv_nsec;
		}
	} else {
		if (pps.tf[1].tv_nsec < pps.tf[2].tv_nsec) {
			v_nsec = pps.tf[1].tv_nsec;	/* 2 1 0 */
			u_nsec = pps.tf[2].tv_nsec - pps.tf[0].tv_nsec;
		} else  if (pps.tf[2].tv_nsec < pps.tf[0].tv_nsec) {
			v_nsec = pps.tf[0].tv_nsec;	/* 1 0 2 */
			u_nsec = pps.tf[1].tv_nsec - pps.tf[2].tv_nsec;
		} else {
			v_nsec = pps.tf[2].tv_nsec;	/* 1 2 0 */
			u_nsec = pps.tf[1].tv_nsec - pps.tf[0].tv_nsec;
		}
	}
#ifdef CONFIG_NTP_PPS_DEBUG
		printk(KERN_DEBUG "v_nsec = %ld, u_nsec = %ld, jitter = %ld\n", v_nsec, u_nsec, pps.jitter);
#endif

	/*
	 * Nominal jitter is due to PPS signal noise and interrupt
	 * latency. If it exceeds the popcorn threshold, the sample is
	 * discarded. otherwise, if so enabled, the time offset is
	 * updated. We can tolerate a modest loss of data here without
	 * much degrading time accuracy.
	 */
	if (unlikely(u_nsec > (pps.jitter << PPS_POPCORN))) {
#ifdef CONFIG_NTP_PPS_DEBUG
		printk(KERN_INFO
		       "hardpps: PPSJITTER: jitter=%ld, limit=%ld\n",
		       u_nsec, (pps.jitter << PPS_POPCORN));
#endif
		time_status |= STA_PPSJITTER;
		pps.jitcnt++;
	} else if (time_status & STA_PPSTIME) {
		time_monitor = -v_nsec;
		L_LINT(time_offset, time_monitor);
		time_adjust = 0;	/* cancel running adjtime() */
#ifdef CONFIG_NTP_PPS_DEBUG
		printk(KERN_DEBUG
		       "hardpps: PPSTIME monitor=%ld, jitter=%ld\n",
		       time_monitor, pps.jitter);
#endif
	}
	pps.jitter += (u_nsec - pps.jitter) >> PPS_FAVG;
	u_sec = pps.tf[0].tv_sec - pps.lastsec;
	/* (The first pulse after a pause will always end a calibration
	 * interval.  The workaround is probably not worth the extra cycles.)
	 */
	if (likely(u_sec < (1 << pps.shift)))
		goto done;
	/*
	 * At the end of the calibration interval the difference between
	 * the first and last counter values becomes the scaled
	 * frequency. It will later be divided by the length of the
	 * interval to determine the frequency update. If the frequency
	 * exceeds a sanity threshold, or if the actual calibration
	 * interval is not equal to the expected length, the data are
	 * discarded. We can tolerate a modest loss of data here without
	 * degrading frequency accuracy.
	 */
	pps.calcnt++;
	v_nsec = -pps.fcount;
	pps.lastsec = pps.tf[0].tv_sec;
	pps.fcount = 0;
	u_nsec = MAXFREQ << pps.shift;
	if (unlikely(v_nsec > u_nsec || v_nsec < -u_nsec ||
		     u_sec != (1 << pps.shift))) {
#ifdef CONFIG_NTP_PPS_DEBUG
		printk(KERN_NOTICE
		       "hardpps: PPSERROR limit=%ld, fcount=%ld, len=%ld\n",
		       u_nsec, v_nsec, u_sec);
#endif
		time_status |= STA_PPSERROR;
		pps.errcnt++;
		/* not in reference implementation */
		if (--pps.intcnt <= -4) {
			pps.intcnt = -4;
			if (pps.shift > PPS_FAVG) {
				pps.shift--;
				pps.intcnt = 0;
			}
		}
		goto done;
	}

	/*
	 * Here the raw frequency offset and wander (stability) is
	 * calculated. If the wander is less than the wander threshold
	 * for four consecutive averaging intervals, the interval is
	 * doubled; if it is greater than the threshold for four
	 * consecutive intervals, the interval is halved. The scaled
	 * frequency offset is converted to frequency offset. The
	 * stability metric is calculated as the average of recent
	 * frequency changes, but is used only for performance
	 * monitoring.
	 */
	L_LINT(ftemp, v_nsec);
	L_RSHIFT(ftemp, pps.shift);
	L_SUB(ftemp, pps.freq);
	u_nsec = L_GINT(ftemp);
	fr_upd = 0;
#ifdef CONFIG_NTP_PPS_DEBUG
	printk(KERN_INFO "hardpps: new frequency %ld >> %d == %ld\n",
	       v_nsec, pps.shift, v_nsec >> pps.shift);
#endif
	/* Not in reference implementation: Use dynamic limit (i.e. stability)
	 * for maximum wander. Approximate SCALE_PPM (65536 / 1000) by 64.
	 */
	v_nsec = (pps.stabil >> 6) << PPS_POPCORN;
	if (unlikely(v_nsec == 0 || v_nsec > PPS_MAXWANDER))
		v_nsec = PPS_MAXWANDER;
#ifdef CONFIG_NTP_PPS_DEBUG
	printk(KERN_DEBUG "hardpps: frequency change=%ld (limit %ld)\n",
	       u_nsec, v_nsec);
#endif
	if (unlikely(u_nsec > v_nsec || u_nsec < -v_nsec)) {
#ifdef CONFIG_NTP_PPS_DEBUG
		printk(KERN_NOTICE
		       "hardpps: PPSWANDER: change=%ld, limit=%ld, shift=%d\n",
		       u_nsec, v_nsec, pps.shift);
#endif
		L_LINT(ftemp, u_nsec > 0 ? v_nsec : -v_nsec);
		time_status |= STA_PPSWANDER;
		pps.stbcnt++;
		if (--pps.intcnt <= -4) {
			pps.intcnt = -4;
			if (pps.shift > PPS_FAVG) {
				pps.shift--;
				pps.intcnt = 0;
			}
		}
	} else {	/* good sample */
		/* Update frequency error estimate in rather stable
                   situations.
		*/
		if (pps.shift > PPS_FAVG + 1)
			fr_upd = 1;
		if (++pps.intcnt >= 4) {
			pps.intcnt = 4;
			if (pps.shift < pps.shiftmax) {
				pps.shift++;
				pps.intcnt = 0;
			}
		}
	}
	if (u_nsec < 0)
		u_nsec = -u_nsec;
	pps.stabil += (u_nsec * SCALE_PPM - pps.stabil) >> PPS_FAVG;

	/*
	 * The PPS frequency is recalculated and clamped to the maximum
	 * MAXFREQ. If enabled, the system clock frequency is updated as
	 * well.
	 */
	L_ADD(pps.freq, ftemp);
	u_nsec = L_GINT(pps.freq);
	if (unlikely(u_nsec > MAXFREQ))
		L_LINT(pps.freq, MAXFREQ);
	else if (unlikely(u_nsec < -MAXFREQ))
		L_LINT(pps.freq, -MAXFREQ);
	else if (fr_upd) {
		/* pps.frange is the maximum difference of the
		   observed frequency values including (in-)stability.
		   Otherwise it's a pessimistic worst-case estimate.
		*/
		if (unlikely(u_nsec - (pps.stabil >> 7) < pps.fmin))
			pps.fmin = u_nsec - (pps.stabil >> 7);
		if (unlikely(u_nsec + (pps.stabil >> 7) > pps.fmax))
			pps.fmax = u_nsec + (pps.stabil >> 7);
		pps.frange = pps.fmax - pps.fmin;
		if (unlikely(pps.frange > MAXFREQ)) {
			printk(KERN_WARNING
			       "hardpps: pps.frange %ld exceeded MAXFREQ\n",
			       pps.frange);
#ifdef CONFIG_NTP_PPS_DEBUG
			printk(KERN_INFO "hardpps: frange=%ld (%ld:%ld)\n",
			       pps.frange, pps.fmin, pps.fmax);
#endif
			/* force recalculation on next update */
			pps.fmin = MAXFREQ;
			pps.fmax = -MAXFREQ;
			pps.frange = MAXFREQ;
		}
	}
#ifdef CONFIG_NTP_PPS_DEBUG
	printk(KERN_INFO "hardpps: new pps.freq=%ld (add %ld)\n",
	       L_GINT(pps.freq), L_GINT(ftemp));
#endif
	if ((time_status & STA_PPSFREQ) != 0 &&
	    (time_status & STA_FREQHOLD) == 0)
	{
		time_freq = pps.freq;
#if 0 /*XXX: MISSING */
		update_nanoscale(tick_nsec * hz - NSEC_PER_SEC +
				 L_GINT(time_freq));
#endif
	}
done:
	write_sequnlock_irqrestore(&xtime_lock, flags);
}

EXPORT_SYMBOL(hardpps);
#endif	/* CONFIG_NTP_PPS */

/*
 * hardupdate() - local clock update
 *
 * This routine is called by adjtimex() to update the local clock
 * phase and frequency. The implementation is of an adaptive-parameter,
 * hybrid phase/frequency-lock loop (PLL/FLL). The routine computes new
 * time and frequency offset estimates for each call. If the kernel PPS
 * discipline code is configured (CONFIG_NTP_PPS), the PPS signal itself
 * determines the new time offset, instead of the calling argument.
 * Presumably, calls to ntp_adjtime() occur only when the caller
 * believes the local clock is valid within some bound (+-128 ms with
 * NTP). If the caller's time is far different than the PPS time, an
 * argument will ensue, and it's not clear who will lose.
 *
 * For uncompensated quartz crystal oscillators and nominal update
 * intervals less than 256 s, operation should be in phase-lock mode,
 * where the loop is disciplined to phase. For update intervals greater
 * than 1024 s, operation should be in frequency-lock mode, where the
 * loop is disciplined to frequency. Between 256 s and 1024 s, the mode
 * is selected by the STA_MODE status bit.
 */
static inline void hardupdate(long offset)
{
	long mtemp;
	l_fp ftemp;

	/*
	 * Select how the phase is to be controlled and from which
	 * source. If the PPS signal is present and enabled to
	 * discipline the time, the PPS offset is used; otherwise, the
	 * argument offset is used.
	 */
	if (!(time_status & STA_PLL))
		return;
	if (!(time_status & STA_PPSTIME && time_status & STA_PPSSIGNAL)) {
		if (offset > MAXPHASE)
			time_monitor = MAXPHASE;
		else if (offset < -MAXPHASE)
			time_monitor = -MAXPHASE;
		else
			time_monitor = offset;
		L_LINT(time_offset, time_monitor);
	}
 
	/*
	 * Select how the frequency is to be controlled and in which
	 * mode (PLL or FLL). If the PPS signal is present and enabled
	 * to discipline the frequency, the PPS frequency is used;
	 * otherwise, the argument offset is used to compute it.
	 */
	if (time_status & STA_PPSFREQ && time_status & STA_PPSSIGNAL) {
		time_reftime = xtime.tv_sec;
		return;
	}
	if (time_status & STA_FREQHOLD || time_reftime == 0)
		time_reftime = xtime.tv_sec;
	mtemp = xtime.tv_sec - time_reftime;
	L_LINT(ftemp, time_monitor);
	L_RSHIFT(ftemp, (SHIFT_PLL + 2 + time_constant) << 1);
	L_MPY(ftemp, mtemp);
	L_ADD(time_freq, ftemp);
	time_status &= ~STA_MODE;
	if (mtemp >= MINSEC && (time_status & STA_FLL || mtemp > MAXSEC)) {
		L_LINT(ftemp, (time_monitor << 4) / mtemp);
		L_RSHIFT(ftemp, SHIFT_FLL + 4);
		L_ADD(time_freq, ftemp);
		time_status |= STA_MODE;
	}
	time_reftime = xtime.tv_sec;
	if (L_GINT(time_freq) > MAXFREQ)
		L_LINT(time_freq, MAXFREQ);
	else if (L_GINT(time_freq) < -MAXFREQ)
		L_LINT(time_freq, -MAXFREQ);
#if 0 /*XXX: MISSING */
	update_nanoscale(tick_nsec * hz - NSEC_PER_SEC + L_GINT(time_freq));
#endif
}

/* adjtimex() mainly allows reading (and writing, if superuser) of
 * kernel time-keeping variables. Used by NTP.
 */
int do_adjtimex(struct timex *txc)
{
        long save_adjust;
	int result;

	/* In order to modify anything, you gotta be super-user! */
	if (txc->modes && !capable(CAP_SYS_TIME))
		return -EPERM;
		
#if 1	/* complain about mode bits that aren't implemented */
	if (unlikely(txc->modes & ~ADJ_SERVED_MODE_BITS)) {
		static int complain = 7;
		if (complain > 0) {
			--complain;
			printk(KERN_NOTICE
			       "adjtimex: %s used unsupported ADJ bits 0x%x\n",
			       current->comm,
			       txc->modes & ~ADJ_SERVED_MODE_BITS);
		}
	}
#endif
	/* Now we validate the data before disabling interrupts */

	if ((txc->modes & ADJ_ADJTIME) == ADJ_ADJTIME)
		/* adjtime() must not be used with any other mode bits */
		if (txc->modes != ADJ_ADJTIME)
			return -EINVAL;

	if (txc->modes != ADJ_ADJTIME && (txc->modes & MOD_OFFSET))
		/* adjustment Offset limited to +- .512 seconds */
		if (txc->offset <= -MAXPHASE || txc->offset >= MAXPHASE)
			return -EINVAL;	

	/* if the clock's frequency is off by more than 10% something is VERY
	 * wrong!
	 */
	if (txc->modes & ADJ_TIMETICK)
		if (unlikely(txc->tick <  900000/USER_HZ ||
			     txc->tick > 1100000/USER_HZ))
			return -EINVAL;

#if 1	/* Map old method to new one; this will vanish soon */
#define	OLD_ADJTIME	(MOD_OFFSET|MOD_CLKA)
	if (unlikely((txc->modes & OLD_ADJTIME) == OLD_ADJTIME)) {
		static int complain = 7;
		txc->modes ^= OLD_ADJTIME;
		txc->modes |= ADJ_ADJTIME;
		if (complain > 0) {
			--complain;
			printk(KERN_WARNING
			       "adjtime: %s used obsolete "
			       "ADJ_OFFSET_SINGLESHOT instead of ADJ_ADJTIME\n",
			       current->comm);
		}
	}
#undef	OLD_ADJTIME
#if 1	/* Establish extra warnings for older applications */
#define	OLD_ADJTICK	(MOD_CLKB)
	if (unlikely((txc->modes & OLD_ADJTICK) == OLD_ADJTICK)) {
		static int complain = 5;
		if (complain > 0) {
			--complain;
			printk(KERN_NOTICE
			       "adjtimex: %s may be using obsolete ADJ_TICK\n",
			       current->comm);
		}
	}
#undef	OLD_ADJTICK
#define	OLD_ADJTICKADJ	(MOD_NANO)
	if (unlikely((txc->modes & OLD_ADJTICKADJ) == OLD_ADJTICKADJ)) {
		static int complain = 5;
		if (complain > 0) {
			--complain;
			printk(KERN_NOTICE
			       "adjtimex: %s could be using obsolete ADJ_TICKADJ\n",
			       current->comm);
		}
	}
#undef	OLD_ADJTICKADJ
#endif	/* print extra warnings */
#endif	/* map old bits to new ones with warning */
	if (unlikely((txc->modes & MOD_OFFSET) != 0 &&
		     (txc->modes & ADJ_ADJTIME) != 0)) {
		/* ntp_adjtime() and adjtime() are mutually exclusive! */
		return -EINVAL;
	}
	write_seqlock_irq(&xtime_lock);
	result = time_state;	/* mostly `TIME_OK' */

#if 0	/* STA_CLOCKERR is never set yet */
	time_status &= ~STA_CLOCKERR;		/* reset STA_CLOCKERR */
#endif
	/* If there are input parameters, then process them */
	if (txc->modes)
	{
		if (txc->modes & MOD_STATUS) {  /* only modify allowed bits */
			if ((time_status & STA_PLL) != 0 &&
			    (txc->status & STA_PLL) == 0) {
				/* If the STA_PLL bit in the status word is
				 * cleared, the state and status words are
				 * reset to the initial values.
				 */
				time_state = TIME_OK;
				time_status = STA_UNSYNC;
#ifdef CONFIG_NTP_PPS
				/* cause an early termination of the PPS
				 * calibration interval (i.e. reset status)
				 */
				pps.shift = PPS_FAVG;
				pps.intcnt = 0;
#endif
			}
			time_status = (txc->status & ~STA_RONLY) |
				      (time_status & STA_RONLY);
		}

		if (txc->modes & MOD_FREQUENCY) {	/* p. 22 */
			long freq;		/* frequency ns/s) */
			freq = txc->freq / SCALE_PPM;
			if (unlikely(freq > MAXFREQ)) {
				result = -EINVAL;
				freq = MAXFREQ;
			} else if (unlikely(freq < -MAXFREQ)) {
				result = -EINVAL;
				freq = -MAXFREQ;
			}
			L_LINT(time_freq, freq);
#if 0 /*XXX: MISSING */
			update_nanoscale(tick_nsec * hz - NSEC_PER_SEC +
					 L_GINT(time_freq));
#endif /*XXX: MISSING */
#ifdef CONFIG_NTP_PPS
			pps.freq = time_freq;
#endif
		}

		if (txc->modes & MOD_MAXERROR) {
			txc->maxerror *= 1000;	/* convert to ns */
			if (unlikely(txc->maxerror < 0 ||
				     txc->maxerror >= NTP_PHASE_LIMIT)) {
				result = -EINVAL;
				txc->maxerror = NTP_PHASE_LIMIT;
			}
			time_maxerror = txc->maxerror;
		}

		if (txc->modes & MOD_ESTERROR) {
			txc->esterror *= 1000;
			if (unlikely(txc->esterror < 0 ||
				     txc->esterror >= NTP_PHASE_LIMIT)) {
				result = -EINVAL;
				txc->esterror = NTP_PHASE_LIMIT;
			}
			time_esterror = txc->esterror;
		}

		/* Note that the timex.constant structure member has a
		 * dual purpose to set the time constant and to set
		 * the TAI offset.
		 */
		if (txc->modes & MOD_TIMECONST) {	/* p. 24 */
			if (unlikely(txc->constant < 0)) {
				result = -EINVAL;
				txc->constant = 0;
			} else if (unlikely(txc->constant > MAXTC)) {
				result = -EINVAL;
				txc->constant = MAXTC;
			}
			time_constant = txc->constant;
		}
		if (txc->modes & MOD_TAI) {
			if (likely(txc->constant > 0))
				time_tai = txc->constant;
			else
				result = -EINVAL;
		}
#ifdef CONFIG_NTP_PPS
		if (txc->modes & MOD_PPSMAX) {
			if (unlikely(txc->shift < PPS_FAVG)) {
				pps.shiftmax = PPS_FAVG;
				result = -EINVAL;
			} else if (unlikely(txc->shift > PPS_FAVGMAX)) {
				pps.shiftmax = PPS_FAVGMAX;
				result = -EINVAL;
			} else
				pps.shiftmax = txc->shift;
			if (pps.shift > pps.shiftmax) {	/* enforce limit */
				pps.shift = pps.shiftmax;
			}
		}
#endif
		if (txc->modes & MOD_NANO)
			time_status |= STA_NANO;
		if (txc->modes & MOD_MICRO)
			time_status &= ~STA_NANO;
		if (txc->modes & MOD_CLKB)
			time_status |= STA_CLK;
		if (txc->modes & MOD_CLKA)
			time_status &= ~STA_CLK;
		if (txc->modes & MOD_OFFSET) {
			if ((time_status & STA_NANO) == 0) {
				/* need nanoseconds, but avoid sign overflow */
				if (txc->offset > MAXPHASE / 1000)
					txc->offset = MAXPHASE;
				else if (txc->offset < -MAXPHASE / 1000)
					txc->offset = -MAXPHASE;
				else
					txc->offset *= 1000;
			}
			/* offset will be clamped silently in hardupdate() */
			hardupdate(txc->offset);
		}
		if (txc->modes == ADJ_ADJTIME) {
			/*
			 * The emulation of adjtime() is actually broken,
			 * because only one ``long'' is available to represent
			 * a ``struct timeval''.
			 */
			/* Save current adjustment to be retuned */
			save_adjust = time_adjust;
			time_adjust = txc->offset;
			if (txc->offset < -30000000 || txc->offset > 30000000)
				printk(KERN_WARNING
				       "adjtime() fails with large offsets\n");
		}
		if (txc->modes & ADJ_TIMETICK) {
			if ((time_status & STA_NANO) == 0)
				txc->tick *= 1000;
			tick_usec = txc->tick;
			tick_nsec = TICK_USEC_TO_NSEC(tick_usec);
#if 0 /*XXX: MISSING */
			update_nanoscale(tick * hz - NSEC_PER_SEC +
					 L_GINT(time_freq));
#endif
		}
		if (txc->modes & ADJ_TICKADJ) {
			/* Traditional UN*Xes seem to allow about ``tick/10'',
			 * but we will allow up to ``tick_usec/4'' (scaled).
			 * Larger values might cause trouble!
			 */
			if (unlikely(txc->tickadj <= 0 ||
				     txc->tickadj > tick_usec / 4)) {
				result = -EINVAL;
				goto leave;
			}
			tickadj = txc->tickadj;
		}
	} /* txc->modes */
leave:
	if (txc->modes & ADJ_ADJTIME)
		txc->offset = save_adjust;
	else {
		if (time_status & STA_NANO)
			txc->offset = time_monitor;
		else
			txc->offset = time_monitor / 1000;
	}
	txc->freq	   = L_GINT(time_freq) * SCALE_PPM;
	txc->maxerror	   = time_maxerror / 1000;
	txc->esterror	   = time_esterror / 1000;
	txc->tai	   = time_tai;
	txc->status	   = time_status;
	txc->constant	   = time_constant;
	if (time_status & STA_NANO) {
		txc->precision = time_precision ? : 1;
		txc->tick = tick_usec;
	} else {
		txc->time.tv_nsec += 500;	/* make microseconds */
		txc->time.tv_nsec /= 1000;
		txc->precision = ((time_precision + 500) / 1000) ? : 1;
		txc->tick = tick_nsec;
	}
	txc->tolerance = MAXFREQ * SCALE_PPM;
#ifdef CONFIG_NTP_PPS
	txc->ppsfreq	   = L_GINT(pps.freq) * SCALE_PPM;
	if (time_status & STA_NANO)
		txc->jitter = pps.jitter;
	else
		txc->jitter = pps.jitter / 1000;
	txc->shift	   = pps.shift;
	txc->stabil	   = pps.stabil;
	txc->jitcnt	   = pps.jitcnt;
	txc->calcnt	   = pps.calcnt;
	txc->errcnt	   = pps.errcnt;
	txc->stbcnt	   = pps.stbcnt;
#endif
	txc->tickadj	   = tickadj;	/* extension by UW */

	if ((time_status & (STA_UNSYNC|STA_CLOCKERR)) != 0
	    /* clock not synchronized (hardware or software error) */
#ifdef CONFIG_NTP_PPS
	    || ((time_status & (STA_PPSFREQ|STA_PPSTIME)) != 0
		&& (time_status & STA_PPSSIGNAL) == 0)
	    /* PPS signal lost when either PPS time or PPS frequency
	     * synchronization requested
	     */
	    || ((time_status & (STA_PPSTIME|STA_PPSJITTER))
		== (STA_PPSTIME|STA_PPSJITTER))
	    /* PPS jitter exceeded when PPS time synchronization requested */
	    || ((time_status & STA_PPSFREQ) != 0
		&& (time_status & (STA_PPSWANDER|STA_PPSERROR)) != 0)
	    /* PPS wander exceeded or calibration error when PPS frequency
	     * synchronization requested
	     */
#endif
	   )
		result = TIME_ERROR;
	
	write_sequnlock_irq(&xtime_lock);
	getnstimeofday(&txc->time);
	if ((time_status & STA_NANO) == 0)
		txc->time.tv_nsec /= 1000;
	notify_arch_cmos_timer();
#ifdef	CONFIG_NTP_DEBUG
	printk(KERN_DEBUG "adjtimex(0x%x) returns %d\n", txc->modes, result);
#endif
	return(result);
}

asmlinkage long sys_adjtimex(struct timex __user *txc_p)
{
	struct timex txc;		/* Local copy of parameter */
	int ret;

	/* Copy the user data space into the kernel copy
	 * structure. But bear in mind that the structures
	 * may change
	 */
	if(copy_from_user(&txc, txc_p, sizeof(struct timex)))
		return -EFAULT;
	ret = do_adjtimex(&txc);
	return copy_to_user(txc_p, &txc, sizeof(struct timex)) ? -EFAULT : ret;
}

inline struct timespec current_kernel_time(void)
{
        struct timespec now;
        unsigned long seq;

	do {
		seq = read_seqbegin(&xtime_lock);
		
		now = xtime;
	} while (read_seqretry(&xtime_lock, seq));

	return now; 
}

EXPORT_SYMBOL(current_kernel_time);

/**
 * current_fs_time - Return FS time
 * @sb: Superblock.
 *
 * Return the current time truncated to the time granularity supported by
 * the fs.
 */
struct timespec current_fs_time(struct super_block *sb)
{
	struct timespec now = current_kernel_time();
	return timespec_trunc(now, sb->s_time_gran);
}
EXPORT_SYMBOL(current_fs_time);

/**
 * timespec_trunc - Truncate timespec to a granularity
 * @t: Timespec
 * @gran: Granularity in ns.
 *
 * Truncate a timespec to a granularity. gran must be smaller than a second.
 * Always rounds down.
 *
 * This function should be only used for timestamps returned by
 * current_kernel_time() or CURRENT_TIME, not with do_gettimeofday() because
 * it doesn't handle the better resolution of the later.
 */
struct timespec timespec_trunc(struct timespec t, unsigned gran)
{
	/*
	 * Division is pretty slow so avoid it for common cases.
	 * Currently current_kernel_time() never returns better than
	 * jiffies resolution. Exploit that.
	 */
	if (gran <= jiffies_to_usecs(1) * 1000) {
		/* nothing */
	} else if (gran == 1000000000) {
		t.tv_nsec = 0;
	} else {
		t.tv_nsec -= t.tv_nsec % gran;
	}
	return t;
}
EXPORT_SYMBOL(timespec_trunc);

/* get system time with nanosecond accuracy */
void getnstimeofday (struct timespec *tv)
{
	unsigned long seq, nsec, sec;
#ifdef CONFIG_TIME_INTERPOLATION
	do {
		seq = read_seqbegin(&xtime_lock);
		sec = xtime.tv_sec;
		nsec = xtime.tv_nsec + time_interpolator_get_offset();
	} while (unlikely(read_seqretry(&xtime_lock, seq)));

	while (unlikely(nsec >= NSEC_PER_SEC)) {
		nsec -= NSEC_PER_SEC;
		++sec;
	}
#else
	{	/*FIXME: Try not to loose nanoseconds */
		struct timeval tv;

		do_gettimeofday(&tv);
		sec = tv.tv_sec;
		nsec = tv.tv_usec * 1000 + xtime.tv_nsec % 1000;
	}
#endif
	tv->tv_sec = sec;
	tv->tv_nsec = nsec;
}
EXPORT_SYMBOL_GPL(getnstimeofday);

#ifdef CONFIG_TIME_INTERPOLATION
/* this is a mess: there are also architecture-dependent ``do_gettimeofday()''
 * and ``do_settimeofday()''
 */
int do_settimeofday (struct timespec *tv)
{
	time_t wtm_sec, sec = tv->tv_sec;
	long wtm_nsec, nsec = tv->tv_nsec;

	if ((unsigned long)tv->tv_nsec >= NSEC_PER_SEC)
		return -EINVAL;

	write_seqlock_irq(&xtime_lock);
	{
		wtm_sec  = wall_to_monotonic.tv_sec + (xtime.tv_sec - sec);
		wtm_nsec = wall_to_monotonic.tv_nsec + (xtime.tv_nsec - nsec);

		set_normalized_timespec(&xtime, sec, nsec);
		set_normalized_timespec(&wall_to_monotonic, wtm_sec, wtm_nsec);
		set_ntp_unsync();		/* reset NTP state */
		time_interpolator_reset();
	}
	write_sequnlock_irq(&xtime_lock);
	clock_was_set();
	return 0;
}
EXPORT_SYMBOL(do_settimeofday);

void do_gettimeofday (struct timeval *tv)
{
	struct timespec ts;

	getnstimeofday(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = (ts.tv_nsec + 500) / 1000;
}

EXPORT_SYMBOL(do_gettimeofday);
#endif

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines were long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
unsigned long
mktime(const unsigned int year0, const unsigned int mon0,
       const unsigned int day, const unsigned int hour,
       const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}

EXPORT_SYMBOL(mktime);

/**
 * set_normalized_timespec - set timespec sec and nsec parts and normalize
 *
 * @ts:		pointer to timespec variable to be set
 * @sec:	seconds to set
 * @nsec:	nanoseconds to set
 *
 * Set seconds and nanoseconds field of a timespec variable and
 * normalize to the timespec storage format
 *
 * Note: The tv_nsec part is always in the range of
 * 	0 <= tv_nsec < NSEC_PER_SEC
 * For negative values only the tv_sec field is negative !
 */
void set_normalized_timespec(struct timespec *ts, time_t sec, long nsec)
{
	while (nsec >= NSEC_PER_SEC) {
		nsec -= NSEC_PER_SEC;
		++sec;
	}
	while (nsec < 0) {
		nsec += NSEC_PER_SEC;
		--sec;
	}
	ts->tv_sec = sec;
	ts->tv_nsec = nsec;
}

/**
 * ns_to_timespec - Convert nanoseconds to timespec
 * @nsec:       the nanoseconds value to be converted
 *
 * Returns the timespec representation of the nsec parameter.
 */
struct timespec ns_to_timespec(const s64 nsec)
{
	struct timespec ts;

	if (!nsec)
		return (struct timespec) {0, 0};

	ts.tv_sec = div_long_long_rem_signed(nsec, NSEC_PER_SEC, &ts.tv_nsec);
	if (unlikely(nsec < 0))
		set_normalized_timespec(&ts, ts.tv_sec, ts.tv_nsec);

	return ts;
}

/**
 * ns_to_timeval - Convert nanoseconds to timeval
 * @nsec:       the nanoseconds value to be converted
 *
 * Returns the timeval representation of the nsec parameter.
 */
struct timeval ns_to_timeval(const s64 nsec)
{
	struct timespec ts = ns_to_timespec(nsec);
	struct timeval tv;

	tv.tv_sec = ts.tv_sec;
	tv.tv_usec = (suseconds_t) ts.tv_nsec / 1000;

	return tv;
}

#if (BITS_PER_LONG < 64)
u64 get_jiffies_64(void)
{
	unsigned long seq;
	u64 ret;

	do {
		seq = read_seqbegin(&xtime_lock);
		ret = jiffies_64;
	} while (read_seqretry(&xtime_lock, seq));
	return ret;
}

EXPORT_SYMBOL(get_jiffies_64);
#endif

EXPORT_SYMBOL(jiffies);

/*
 * timevar_init() - initialize variables and structures
 * (needed before first time interrupt is processed)
 *
 * This routine must be called after the kernel variables hz and tick
 * are set or changed and before the next tick interrupt. In this
 * particular implementation, these values are assumed set elsewhere in
 * the kernel. The design allows the clock frequency and tick interval
 * to be changed while the system is running. So, this routine should
 * probably be integrated with the code that does that.
 */
void timevar_init(void)
{
	struct timespec ts;

#ifdef	CONFIG_NTP_DEBUG
	printk(KERN_DEBUG "PPSkit DEBUG: timevar_init() executed\n");
#endif
	/*
	 * The following variable must be initialized any time the
	 * kernel variable hz is changed.
	 */
	tick_nsec = (NSEC_PER_SEC + hz/2) / hz;
	tick_usec = (tick_nsec + 500) / 1000;
	/* Speed of adjtime() is 50ms/s; may not be <= 0 */
	tickadj = 50000/hz ? : 1;
	time_adjust = 0;
	rtc_update = 660;			/* update every 11 minutes */

	/*
	 * The following variables are initialized only at startup. Only
	 * those structures not cleared by the compiler need to be
	 * initialized, and these only in the simulator. In the actual
	 * kernel, any nonzero values here will quickly evaporate.
	 */
	time_state = TIME_OK;
	time_status = STA_UNSYNC;
	time_tai = 0;
	time_maxerror = time_esterror = NTP_PHASE_LIMIT;
	L_CLR(time_offset);
	L_CLR(time_freq);
	L_LINT(time_adj, NSEC_PER_SEC);		/* preset for first second */
	L_CLR(time_phase);
#if 0 /*FIXME: we don't have it yet (UW)*/
	sys_clock_getres(CLOCK_REALTIME, &ts);
	time_precision = ts.tv_nsec;
#else
	time_precision = 1;
#endif
#ifdef CONFIG_NTP_PPS
	pps.shift = PPS_FAVG;
	pps.shiftmax = PPS_FAVGDEF;
	pps.tf[0].tv_sec = pps.tf[0].tv_nsec = 0;
	pps.tf[1].tv_sec = pps.tf[1].tv_nsec = 0;
	pps.tf[2].tv_sec = pps.tf[2].tv_nsec = 0;
	pps.fcount = 0;
	L_CLR(pps.freq);
	pps.fmin = MAXFREQ;
	pps.fmax = -MAXFREQ;
	pps.frange = MAXFREQ;
#endif
}
