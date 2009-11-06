/* daemonize.c
	$Id$
	Glen Wiley, <gwiley@gwiley.com>

	TODO: some implementations may call setpgrp with args (UNPv1)
	TODO: BSD may want us to call wait3() on child signals (UNPv1)
	TODO: regression tests
*/

#if HAVE_CONFIG_H
//# include "config.h"
#include <lofar_config.h>
#endif

#include <errno.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#if HAVE_SIGNAL_H
# include <signal.h>
#endif

/* the umask gets set to this */
static const mode_t NEWUMASK= S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH 
 | S_IXOTH;

/*---------------------------------------- daemonize 
	call this function to safely make the current process a daemon
	detects whether we were started from init - NOTE: this test is
	unreliable, if the parent dies VERY quickly then we will mistakenly
	think we started from init
	if nofork != 0 then do not try to fork at all
	returns 0 on success, -1 or errno on failure
*/
int daemonize(int nofork)
{
	pid_t    pid;
	int      i;
	struct   rlimit rlim;
#if HAVE_SIGACTION
	struct   sigaction sact;
	sigset_t sset;
#endif

	/* get into the background if not started by init */
	if(nofork != 0 || getppid() != 1)
	{
		pid = fork();
		if(pid == -1)
			return errno;
		if(pid != 0)
			exit(0);

		/* dissociate from process group and control terminal */
#if HAVE_SETSID
		setsid();
#else
# if HAVE_SETPGRP
		setpgrp();
# endif
#endif

#if HAVE_SIGACTION
		sigemptyset(&sset);
		sact.sa_mask    = sset;
		sact.sa_handler = SIG_IGN;
		if(sigaction(SIGHUP, &sact, NULL) != 0)
			return errno;
#else
		if(signal(SIGHUP, SIG_IGN) != 0)
			return errno;
#endif

		pid = fork();
		if(pid == -1)
			return errno;
		if(pid != 0)
			exit(0);

		/* ignore terminal io signals */
#if HAVE_SIGACTION
		if(sigaction(SIGTTIN, &sact, NULL) != 0)
			return errno;
		if(sigaction(SIGTTOU, &sact, NULL) != 0)
			return errno;
		if(sigaction(SIGTSTP, &sact, NULL) != 0)
			return errno;
		if(sigaction(SIGCLD, &sact, NULL) != 0)
			return errno;
#else
		if(signal(SIGTTIN, SIG_IGN) != 0)
			return errno;
		if(signal(SIGTTOU, SIG_IGN) != 0)
			return errno;
		if(signal(SIGTSTP, SIG_IGN) != 0)
			return errno;
		if(signal(SIGCLD, SIG_IGN) != 0)
			return errno;
#endif
	} /* if(getppid() == 1) */

	/* close all file descriptors */
	if(getrlimit(RLIMIT_NOFILE, &rlim) != 0)
		return errno;
	for(i=0; i<rlim.rlim_cur; i++)
		close(i);
	errno = 0;

/* Don't change dir. It bites LOFAR appliations. */
#if 0
	/* change to root directory to free up mounts */
	if(chdir("/") != 0)
		return errno;
#endif

	/* reset file creation mask  to rather restrictive */
	(void)umask(NEWUMASK);

	return 0;
} /* daemonize */

/* daemonize.c */
