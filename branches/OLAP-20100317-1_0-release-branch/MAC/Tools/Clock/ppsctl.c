/* Control program for PPS API v1 (RFC 2783), a bit like `stty'
 *
 * Copyright (c) 1996 - 2001 by Ulrich Windl
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: ppsctl.c,v 1.4 2001/07/07 17:02:44 windl Exp $
 */
#include	<unistd.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<errno.h>
#include	<sys/time.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<sys/utsname.h>

#include	<linux/version.h>

#include	<sys/timepps.h>

#define	ALL_BITS_ON(v, b)		(((v) & (b)) == (b))
#define	ALL_BITS_OFF(v, b)		(((v) & (b)) == 0)
#define	BITS_OFF(v, b)			((v) & ~(b))
#define	BITS_ON(v, b)			((v) | (b))

static	const char	id[] = "$Id: ppsctl.c,v 1.4 2001/07/07 17:02:44 windl Exp $";

#define	BIT(n)	(1UL << (n))
#define OPT_MONITOR_LOOP	BIT(0)	/* monitor loop */

static	unsigned long	options = 0;
static	const char	*format	= "Etdj";	/* output format */

static	char		trigger	= '0';	/* off | assert | clear | both */

static	int		fd	= -1;	/* file descriptor of port */
static	pps_handle_t	ppshandle;	/* PPS handle */

struct clock_info {			/* information about clock and conditions */
	pps_info_t	info;		/* pulse information */
	int		have_assert;	/* assert event detected */
	int		have_clear;	/* clear event detected */
	long		sec, nsec;	/* last time stamp */
	long		dsec, dnsec;	/* difference since last time stamp */
	long		seq, dseq;	/* sequence number and difference */
	long		nano_offset;	/* offset around zero (ns) */
	long		jitter;		/* jitter between pulses (ns) */
};

/* update *cip */
static	int	update_clock_info(struct clock_info *cip)
{
	static struct clock_info	last;
	static struct timespec	sleeper = {0, 0};
	pps_seq_t	ds;
	int	result = -1;

	if ( time_pps_fetch(ppshandle, PPS_TSFMT_TSPEC,
			    &cip->info, &sleeper) == -1 )
	{
		perror("time_pps_fetch()");
		memset(&cip->info, 0, sizeof(cip->info));
	}

	cip->dseq = cip->have_assert = cip->have_clear = 0;
	if ( (ds = cip->info.assert_sequence - last.info.assert_sequence)
	     != 0 ) {
		cip->have_assert = 1;
		cip->dseq += ds;
		cip->sec = cip->info.assert_timestamp.tv_sec;
		cip->nsec = cip->info.assert_timestamp.tv_nsec;
		cip->seq = cip->info.assert_sequence;
	}
	if ( (ds = cip->info.clear_sequence - last.info.clear_sequence)
	     != 0 ) {
		cip->have_clear = 1;
		cip->dseq += ds;
		cip->sec = cip->info.clear_timestamp.tv_sec;
		cip->nsec = cip->info.clear_timestamp.tv_nsec;
		cip->seq = cip->info.clear_sequence;
	}
	cip->dsec = cip->sec - last.sec;
	cip->dnsec = cip->nsec - last.nsec;
	if ( cip->dseq )
	{
		cip->nano_offset = cip->nsec;
		if ( cip->nano_offset > 500000000 )
			cip->nano_offset -= 1000000000;
		if ( cip->dnsec < 0 )
		{
			--cip->dsec;
			cip->dnsec += 1000000000;
		}
		else if ( cip->dnsec > 1000000000 )
		{
			++cip->dsec;
			cip->dnsec -= 1000000000;
		}
		cip->jitter = cip->dsec - last.dsec;
		cip->jitter *= 1000000000;
		cip->jitter += cip->dnsec - last.dnsec;
		last = *cip;
		result = 0;
	}
	return(result);
}

/* monitor PPS API values */
static	void	monitor_loop(struct clock_info *cip)
{
	struct clock_info	last;
	pps_info_t		buf;
	const char		*fmt;
	char			fmtchr;
	const char		*sep;

	while ( 1 )
	{
		if ( time_pps_fetch(ppshandle, PPS_TSFMT_TSPEC,
				    &buf, NULL) != 0 )
		{
			perror("time_pps_fetch()");
			continue;
		}
		update_clock_info(cip);
		if ( cip->dseq == 0 )
			continue;
		sep = "";
		for ( fmt = format; (fmtchr = *fmt) != '\0'; ++fmt )
		{
			if ( fmtchr == 'E' )	/* automagic event */
				fmtchr = cip->have_assert ? 'a' : 'c';
			switch ( fmtchr )
			{
			case 'a':
				printf("%sassert %lu",
				       sep, cip->info.assert_sequence);
				sep = " ";
				break;
			case 'c':
				printf("%sclear %lu",
				       sep, cip->info.clear_sequence);
				sep = " ";
				break;
			case 'd':
				printf("%sdelta %lu.%09lu",
				       sep, cip->dsec, cip->dnsec);
				sep = " ";
				break;
			case 'j':
				printf("%sjitter %ld", sep, cip->jitter);
				sep = " ";
				break;
			case 'l':
				printf("%slevel %c",
				       sep, cip->have_assert ? '1' : '0');
				sep = " ";
				break;
			case 'o':
				printf("%soffset %ld", sep, cip->nano_offset);
				sep = " ";
				break;
			case 't':
				printf("%stime %lu.%09lu",
				       sep, cip->sec, cip->nsec);
				sep = " ";
				break;
			default:
				printf("%s?", sep);
				sep = " ";
				break;
			
			}
		}
		printf("\n");

		last = *cip;
		fflush(stdout);
	}
}

/* provide a usage message */
static	void	usage(void)
{
	fprintf(stderr,
		"Known options are:\n"
		"\t-eX\tEcho - set PPS API echo for `X' events (a|c)\n"
		"\t-Ffmt\tFormat - set output format to `fmt' (see below)\n"
		"\t-m\tmonitor values\n"
		"\t-oXY\tSet PPS API offset for `X' events (a|c) to `Y'ns\n"
		"\t-pZ\tUse port `Z'\n"
		"\t-t{0|a|c|b}[h]\ttrigger for event: off|assert|clear|both\n"
		"\t\t'h' = route to `hardpps()' kernel consumer\n"
		);
	fprintf(stderr,
		"\n"
		"Format is composed of the following components:\n"
		"\tE\tcurrent event's counter\n"
		"\ta\tassert event's counter\n"
		"\tc\tclear event's counter\n"
		"\td\tdelta between event timestamps\n"
		"\tj\tjitter between event time stamps (delta of delta)\n"
		"\tl\tlogical level of event ('0' = clear, '1' = assert)\n"
		"\to\toffset around current second\n"
		"\tt\ttimestamp of current event\n"
		);
}

int	main(int argc, char *argv[])
{
	struct utsname	un;
	struct clock_info ci;
	pps_params_t	parm;
	int		kc_edge;
	int		caps;
	int		ch;

//	printf("(This version has been compiled"
//#ifdef __linux__
//	       " on Linux %s\n", UTS_RELEASE
//#endif
//	       );
//	printf(" using "
//#if !defined(__GNU_LIBRARY__) || __GNU_LIBRARY__ < 6
//	       "an old C library (not glibc-2.x)"
//#else
//	       "glibc-%d.%d",
//	       __GLIBC__, __GLIBC_MINOR__
//#endif
//	       );
	uname(&un);
	printf(". Now running %s %s)\n", un.sysname, un.release);
	parm.mode = PPS_TSFMT_TSPEC;	/* set default flags */
	while ( (ch = getopt(argc, argv, "e:F:mo:p:t:")) != -1 )
	{
		switch ( ch )
		{
		case 'e':
			switch (optarg[0])
			{
			case 'a':
				parm.mode |= PPS_ECHOASSERT;
				break;
			case 'c':
				parm.mode |= PPS_ECHOCLEAR;
				break;
			default:
				fprintf(stderr, "'-%c%s' is illegal\n",
					ch, optarg);
				usage();
				exit(1);
			}
			break;
		case 'F':
			format = optarg;
			break;
		case 'm':	/* monitor jitter only */
			options = BITS_ON(options, OPT_MONITOR_LOOP);
			break;
		case 'o':
			switch (optarg[0])
			{
			case 'a':
				parm.mode |= PPS_OFFSETASSERT;
				parm.assert_offset.tv_sec = 0;
				parm.assert_offset.tv_nsec = strtol(optarg + 1,
								    NULL, 10);
				break;
			case 'c':
				parm.mode |= PPS_OFFSETCLEAR;
				parm.clear_offset.tv_sec = 0;
				parm.clear_offset.tv_nsec = strtol(optarg + 1,
								   NULL, 10);
				break;
			default:
				fprintf(stderr, "'-%c%s' is illegal\n",
					ch, optarg);
				usage();
				exit(1);
			}
			break;
		case 'p':
			if ( (fd = open(optarg, O_RDWR)) == -1 )
			{
				perror(optarg);
				return(1);
			}
			break;
		case 't':	/* select desired edge of the pulse */
			trigger = optarg[0];
			switch ( trigger )
			{
			case '0':
			case 'a':
			case 'c':
			case 'b':
				break;
			default:
				fprintf(stderr, "'-%c%s' is illegal\n",
					ch, optarg);
				usage();
				exit(1);
			}
			if ( optarg[1] == 'h' )
				trigger = toupper(trigger);
			break;
		case '?':
			fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			usage();
			exit(1);
		}
	}
	if ( optind > argc )
	{
		fprintf(stderr, "Extra arguments ignored!\n");
		usage();
		exit(1);
	}

	if ( fd == -1 )
	{
		fprintf(stderr, "Missing port to set up!\n");
		usage();
		exit(1);
	}

	kc_edge = 0;
	switch ( trigger )
	{
	case 'A':
		kc_edge |= PPS_CAPTUREASSERT;	/* fall through */
	case 'a':
		parm.mode |= PPS_CAPTUREASSERT;
		break;
	case 'C':
		kc_edge |= PPS_CAPTURECLEAR;	/* fall through */
	case 'c':
		parm.mode |= PPS_CAPTURECLEAR;
		break;
	case 'B':
		/* parm.mode |= PPS_HARDPPSONBOTH; */	/* fall through */
	case 'b':
		parm.mode |= PPS_CAPTUREBOTH;
		break;
	}

	if ( time_pps_create(fd, &ppshandle) )
	{
		perror("time_pps_create()");
		return(1);
	}
	if ( time_pps_getcap(ppshandle, &caps) )
	{
		perror("time_pps_getcap()");
		return(1);
	}
	printf("PPS API capabilities are 0x%x\n", caps);

	parm.api_version = PPS_API_VERS_1;
	if ( time_pps_setparams(ppshandle, &parm) != 0 )
	{
		perror("time_pps_setparams()");
		fprintf(stderr, "handle=%d, mode=%#04x\n",
			ppshandle, parm.mode);
		/* continue, maybe just monitoring */
	}
	else
	{
		fprintf(stderr,
			"PPS API v%d set up (handle %d, mode %#04x)\n",
			parm.api_version, ppshandle, parm.mode);
		if ( kc_edge != 0 )
		{
			if ( time_pps_kcbind(ppshandle, PPS_KC_HARDPPS,
					     kc_edge, PPS_TSFMT_TSPEC) != 0 )
			{
				perror("time_pps_kcbind()");
				fprintf(stderr, "handle=%d, edge=%#04x\n",
					ppshandle, kc_edge);
				/* continue, maybe just monitoring */
			}
			else
			{
				fprintf(stderr,
					"PPS API kernel consumer HARDPPS bound"
					" to edge %#04x\n",
					kc_edge);
			}
		}
	}
	memset(&ci, 0, sizeof(ci));
	update_clock_info(&ci);
	if ( options & OPT_MONITOR_LOOP )
		monitor_loop(&ci);
	return(0);
}

