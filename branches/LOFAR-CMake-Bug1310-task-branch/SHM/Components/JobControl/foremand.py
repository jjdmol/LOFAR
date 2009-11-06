#! /usr/bin/python

import os
import sys
import pwd
import time
import optparse

# unix specific
import fcntl
import signal
import syslog
import resource

# lofar specific
import lofar.shm

#
# GLOBALS
#

# global quit flag (set by handle_quit())
quit = False
# return code in case of error
EXIT_ERROR = 127
# shm database
db = None
# job control object
jobctrl = None
# mapping from process (group) id's to jobs
processTable = {}

def handle_quit(signum, frame):
    # NOTE: at least on Ubuntu, the same signal is __not__ blocked when executing the
    # Python signal handler. of course, it could be that the signal __is__ blocked while
    # executing the C signal handler, but this handler only schedules a synchronous
    # call to the Python handler and is therefore very fast. As a result, the Python
    # handler can be pre-empted anywhere between byte codes whenever another signal is caught.

    global quit

    # when this is executed from a child process, it has no effect (due to the copy-on-write
    # semantics of fork()).
    quit = True


def handle_exited_jobs():
    # polling is used to detect jobs that have terminated. the event driven approach is also
    # possible using a SIGCHLD handler. however, this would lead to the scenario where the
    # job process terminates before the parent process has had the chance to register the
    # job process's pid in the process table. the SIGCLHD handler will execute first,
    # trying to remove the pid from the process table (which will fail, of course). then
    # the parent process will resume, adding the pid of the already terminated process to the
    # process table. result: an inconsistent process table. the polling approach does
    # not suffer from this drawback, because all updates to the process table occur in the
    # same thread of execution.
    while True:
        # get process exit status of any child process
        try:
            (pid, status) = os.waitpid(-1, os.WNOHANG)
            if pid == 0:
                #MAXMOD
                syslog.syslog(syslog.LOG_DEBUG, "%s():: pid is 0" % (sys._getframe().f_code.co_name))
                break
        except OSError:
            break

        try:
            job = processTable.pop(pid)
        except KeyError:
            syslog.syslog(syslog.LOG_WARNING, "%s():: waitpid() returned information about an unknown child process (pid %i)" % (sys._getframe().f_code.co_name, pid))
        else:
            try:
                if os.WIFEXITED(status):
                    code = os.WEXITSTATUS(status)
                    if code == 0:
                        jobctrl.log(job, lofar.shm.job_control.LOG_NOTICE, "job executed succesfully (return code 0)")
                        syslog.syslog(syslog.LOG_DEBUG, "%s():: job process %i exited normally" % (sys._getframe().f_code.co_name, pid))
                    else:
                        jobctrl.log(job, lofar.shm.job_control.LOG_ERR, "job exited due to an error (return code %i)" % os.WEXITSTATUS(status))
                        syslog.syslog(syslog.LOG_ERR, "%s():: job process %i terminated with return code %i" % (sys._getframe().f_code.co_name, pid, os.WEXITSTATUS(status)))
                else:
                    jobctrl.log(job, lofar.shm.job_control.LOG_WARNING, "job exited due to signal %i" % os.WTERMSIG(status))
                    syslog.syslog(syslog.LOG_WARNING, "%s():: job process %i exited due to signal %i" % (sys._getframe().f_code.co_name, pid, os.WTERMSIG(status)))

                # unbind job (may have already been done, e.g. if the job timed out or was stopped)
                jobctrl.unbind(job)

            except lofar.shm.DatabaseError, ex:
                syslog.syslog(syslog.LOG_ERR, "%s():: unable to communicate with database (%s)" % (sys._getframe().f_code.co_name, ex))


def run_job(job):
    # note: using os._exit() instead of sys.exit() in child processes,
    #     see http://www.python.org/doc/lib/os-process.html
    # note: using os._exit() seems to prevent the child process from closing
    #     the database connection upon exit.

    try:
        pid = os.fork()
    except OSError, ex:
        syslog.syslog(syslog.LOG_ERR, "%s():: unable to fork off a job process (%s)" % (sys._getframe().f_code.co_name, ex))
        return -1

    if pid != 0:
        #
        # parent process
        #

        # establish new process group for child
        try:
            os.setpgid(pid, pid)
        except OSError:
            # this happens when the job process (i.e. our child process) executes
            # the call to execvpe() before we get a change to call setpgid(). the parent
            # cannot change the process group id of a child that has called one of the exec()
            # functions. this is not considered an error, because when this occurs we know for
            # certain that the child has called setpgid() and therefore its process group id is
            # set correctly.
            pass

        return pid

    try:
        #
        # job process
        #

        # POTENTIAL RACE CONDITION
        #
        # handle_quit() is also defined in the address space of the child
        # process! it is therefore possible the child catches a signal here,
        # before it has had the chance to reset the signal dispositions
        # (see below). the current implementation of handle_quit() is
        # robust to calls from child processes, so this is not harmful.
        #
        # NOTE: however, if a signal is caught here, the process will
        # _not_ terminate. this should be no problem, because as long
        # as a timed out process is in the processTable, cycle() will
        # try to kill it.

        # reset signal handlers inherited from parent process
        if signal.getsignal(signal.SIGTERM) != signal.SIG_IGN:
            signal.signal(signal.SIGTERM, signal.SIG_DFL)

        if signal.getsignal(signal.SIGINT) != signal.SIG_IGN:
            signal.signal(signal.SIGINT, signal.SIG_DFL)

        if signal.getsignal(signal.SIGQUIT) != signal.SIG_IGN:
            signal.signal(signal.SIGQUIT, signal.SIG_DFL)

        # establish a new process group with ourself as group leader
        try:
            os.setpgid(0, 0)
        except OSError, ex:
            syslog.syslog(syslog.LOG_ERR, "%s():: unable to establish a new process group (%s)" % (sys._getframe().f_code.co_name, ex))
            return

        # set resource limits
        try:
            for (res, limit) in lofar.shm.config.get("job_control.rlimits").items():
                resource.setrlimit(res, (long(limit), long(limit)))
        except ValueError, ex:
            syslog.syslog(syslog.LOG_ERR, "%s():: cannot set resource limit (%s)" % (sys._getframe().f_code.co_name, ex))
            return

        # close logs
        syslog.syslog(syslog.LOG_DEBUG, "%s():: ready to execute command \"%s\"" % (sys._getframe().f_code.co_name, os.path.join(lofar.shm.config.get("job_control.script_path"), job.command)))
        syslog.closelog()

        # set our environment. NOTE: after setuid() we're no longer root.
        os.setgid(lofar.shm.config.get("job_control.gid"))
        os.setuid(lofar.shm.config.get("job_control.uid"))
        # start job script
        os.chdir(lofar.shm.config.get("job_control.script_path"))
        os.execvpe(lofar.shm.config.get("job_control.shell"), (lofar.shm.config.get("job_control.shell"), "-c", os.path.join(os.curdir, job.command)), {"JOB_ID":str(job.id), "JOB_TICKET_NO":str(job.ticket_no)})
        #MAXMOD
        syslog.syslog(syslog.LOG_NOTICE,"%s():: called execvpe args %s:%s:%s:%s:%s"%(sys._getframe()._f_code.co_name,
                                                                                     lofar.shm.config.get("job_control.shell"),
                                                                                     lofar.shm.config.get("job_control.shell"),
                                                                                     os.path.join(os.curdir,job.command),
                                                                                     str(job.id),str(job.ticket_no)))
    finally:
        os._exit(EXIT_ERROR)


def send_signal(pgids, signal):
    for pgid in pgids:
        syslog.syslog(syslog.LOG_DEBUG, "%s():: sending signal %i to process group %i" % (sys._getframe().f_code.co_name, signal, pgid))

        try:
            os.killpg(pgid, signal)
        except OSError:
            pass


def cycle():

    # check if any jobs have terminated (by calling exit() or due to a signal)
    handle_exited_jobs()

    try:
        load_avg = os.getloadavg()
    except OSError:
        #MAXMOD
        syslog.syslog(syslog.LOG_NOTICE, "%s()(MAXMOD):: could not get load average"% sys._getframe().f_code.co_name)
        pass
    else:
        if load_avg[0] < lofar.shm.config.get("job_control.max_load_avg"):
            try:
                #MAXMOD
                syslog.syslog(syslog.LOG_NOTICE, "%s()(MAXMOD):: about to do jobctrl.bind()"% sys._getframe().f_code.co_name)
                job = jobctrl.bind()
                if job is not None:
                    # succesfully bound a job...
                    syslog.syslog(syslog.LOG_DEBUG, "%s():: bound job %i with ticket number %i" % (sys._getframe().f_code.co_name, job.id, job.ticket_no))

                    # get additional job info (currently only command)
                    jobctrl.get_info(job)

                    # start job process
                    if job.command is not None:
                        pid = run_job(job)

                        if pid >= 0:
                            processTable[pid] = job
                        else:
                            jobctrl.unbind(job)
                    else:
                        jobctrl.unbind(job)
            except lofar.shm.DatabaseError, ex:
                syslog.syslog(syslog.LOG_ERR, "%s():: unable to communicate with the database (%s)" % (sys._getframe().f_code.co_name, ex))
          #MAXMOD
        else: #MAXMOD
            syslog.syslog(syslog.LOG_NOTICE, "%s()(MAXMOD):: load too high" % sys._getframe().f_code.co_name)
    # reschedule any jobs that have timed out
    try:
        jobctrl.update_queue()
    except lofar.shm.DatabaseError, ex:
        syslog.syslog(syslog.LOG_ERR, "%s():: unable to communicate with the database (%s)" % (sys._getframe().f_code.co_name, ex))

    # SIGKILL jobs that have timed out
    timed_out = []
    for pid in processTable:
        job = processTable[pid]

        # check if job is still valid (not stopped or timed out)
        try:
            valid = jobctrl.valid(job)
        except lofar.shm.DatabaseError, ex:
            # be optimistic
            valid = True
            syslog.syslog(syslog.LOG_ERR, "%s():: unable to communicate with the database (%s)" % (sys._getframe().f_code.co_name, ex))

        if not valid:
            syslog.syslog(syslog.LOG_WARNING, "%s():: job %i with ticket number %i was stopped or timed out" % (sys._getframe().f_code.co_name, job.id, job.ticket_no))
            timed_out.append(pid)

    send_signal(timed_out, signal.SIGKILL)


# How to become a daemon
# ----------------------
# See Advanced Programming in the UNIX Environment by Stevens and Rago, section 13.3, pp. 425/426.
# See also the UNIX Programming FAW (http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC16)
#
# 1. call umask(0) to clear the file mode creation mask. the umask is inherited and may be set to
#    deny certain permissions.
# 2. fork() and let the parent exit. this returns control to the shell. also, the child inherits
#    the parent's process group id, but gets its own process id. by definition, it is therefore
#    not a process group leader. this is a prerequisite for calling setsid(), which is the next
#    step.
# 3. call setsid() to create a new session. the process becomes leader of a new sessions, process
#    group leader of a new process group and has no controlling terminal.
# 4. fork() again to ensure the process is not a session leader. (System V systems may allocate a
#    controlling terminal for session leaders under certain conditions).
# 5. chdir("/") to ensure that the daemon does not tie up any mounted file systems.
# 6. close all open file descriptors we may have inherited from our parent.
# 7. re-open stdin, stdout, and stderr; for instance, point stdout and stderr to a log file, or
#    simply redirect to /dev/null.
#
def daemonize():
    try:
        no_file = resource.getrlimit(resource.RLIMIT_NOFILE)
    except ValueError, ex:
        print "error: unable to determine NOFILE resource limit; daemon not started (%s)" % ex
        sys.exit(EXIT_ERROR)

    # 1. clear umask
    try:
        os.umask(0)
    except OSError, ex:
        print "error: unable to set umask; daemon not started %s" % ex
        sys.exit(EXIT_ERROR)

    # 2. fork and let parent exit
    try:
        pid = os.fork()
    except OSError, ex:
        print "error: unable to fork; daemon not started (%s)" % ex
        sys.exit(EXIT_ERROR)

    if pid != 0:
        # parent exits
        # NOTE: Steven and Rago specify a standard exit(0) here, while the UNIX Programming FAQ
        # recommends _exit(0) (see http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC6). We favour
        # the latter approach, because it seems correct to only do user-mode clean-up once.
        os._exit(0)

    #
    #    FIRST CHILD PROCESS
    #

    # 3. create a new session
    try:
        sid = os.setsid()
    except OSError, ex:
        print "error: unable to create a new session; daemon not started (%s)" % ex
        sys.exit(EXIT_ERROR)

    # 4. second fork
    try:
        pid = os.fork()
    except OSError, ex:
        print "error: unable to fork; daemon not started (%s)" % ex
        sys.exit(EXIT_ERROR)

    if pid != 0:
        # parent exits
        # NOTE: Steven and Rago specify a standard exit(0) here, while the UNIX Programming FAQ
        # recommends _exit(0) (see http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC6). We favour
        # the latter approach, because it seems correct to only do user-mode clean-up once.
        os._exit(0)

    #
    #    SECOND CHILD PROCESS
    #

    try:
        # 5. change the working directory
        os.chdir("/")

        print "closing stdout; from this moment on, any errors will be sent to the syslog."

        # 6a. flush stdout and stderr
        sys.stdout.flush()
        sys.stderr.flush()

        # 6b. close all open file descriptors (uses soft NO_FILE limit)
        for fd in range(0, max(no_file[0], 1024)):
            try:
                os.close(fd)
            except OSError:
                pass

        # 7. re-open stdin, stdout, and stderr as /dev/null
        # for python >= 2.4 use os.devnull
        sys.stdin = open("/dev/null", "r")
        sys.stdout = open("/dev/null", "w")
        sys.stderr = open("/dev/null", "w")
    except (IOError, OSError), ex:
        syslog.syslog(syslog.LOG_ERR, "unable to chdir, close open file descriptors, or open /dev/null; daemon not started (%s)" % ex)
        sys.exit(EXIT_ERROR)

    #
    # properly daemonized from now on...
    #


def check_config():

    print "  - cycle time:",

    try:
        try:
            cycle_time = int(lofar.shm.config.get("job_control.cycle_time"))
        except ValueError:
            print "\tinvalid cycle time; please check lofar-shm config file."
            sys.exit(EXIT_ERROR)

        if cycle_time < 1:
            print "\tinvalid cycle time; please check lofar-shm config file."
            sys.exit(EXIT_ERROR)
        elif cycle_time == 1:
            print "\t1 second"
        else:
            print "\t%i seconds" % cycle_time
    except KeyError:
        print "\tnot found; using default cycle time (10 seconds)."
        cycle_time = 10

    lofar.shm.config.set("job_control.cycle_time", cycle_time)


    print "  - max_load_avg:",

    try:
        try:
            max_load_avg = float(lofar.shm.config.get("job_control.max_load_avg"))
        except ValueError:
            print "\tinvalid maximum load average; please check lofar-shm config file."
            sys.exit(EXIT_ERROR)

        if max_load_avg < 0.0:
            print "\tinvalid maximum load average; please check lofar-shm config file."
            sys.exit(EXIT_ERROR)

        print "\t%f" % max_load_avg
    except KeyError:
        print "\tnot found; using default maximum load average (0.8)."
        max_load_avg = 0.8

    lofar.shm.config.set("job_control.max_load_avg", max_load_avg)


    print "  - execute as:",

    try:
        pw_entry = pwd.getpwnam(lofar.shm.config.get("job_control.execute_as"))
        if pw_entry.pw_shell is None:
            print "\tno shell set for user %s, please check /etc/passwd" % (lofar.shm.config.get("job_control.execute_as"),)
            sys.exit(EXIT_ERROR)

        lofar.shm.config.set("job_control.gid", pw_entry.pw_gid)
        lofar.shm.config.set("job_control.uid", pw_entry.pw_uid)
        lofar.shm.config.set("job_control.shell", pw_entry.pw_shell)
        print "\t%s (gid: %i, uid: %i, shell: %s)" % (lofar.shm.config.get("job_control.execute_as"), lofar.shm.config.get("job_control.gid"), lofar.shm.config.get("job_control.uid"), lofar.shm.config.get("job_control.shell"))
    except KeyError:
        print "\tnot found; please check lofar-shm config file."
        sys.exit(EXIT_ERROR)


    print "  - script_path:",

    try:
        if os.path.isdir(lofar.shm.config.get("job_control.script_path")):
            print "\t%s" % (lofar.shm.config.get("job_control.script_path"),)
        else:
            print "\t%s does not exist or is not a directory; please check lofar-shm config file." % (lofar.shm.config.get("job_control.script_path"),)
            sys.exit(EXIT_ERROR)
    except KeyError:
        print "\tnot found; please check lofar-shm config file."
        sys.exit(EXIT_ERROR)


    print "  - rlimits:"

    rlimits = {}
    try:
        for keyval_pair in lofar.shm.config.get("job_control.rlimits").split():
            keyval = keyval_pair.split(':')

            if len(keyval) != 2:
                print "    malformed: %s" % keyval_pair
                sys.exit(EXIT_ERROR)
            else:
                key   = keyval[0].strip().upper()
                value = keyval[1].strip().upper()

                try:
                    rlimit_id = getattr(resource, key)
                except AttributeError:
                    print "    unknown resource: %s" % key
                    sys.exit(EXIT_ERROR)
                else:
                    rlimits[rlimit_id] = value
                    print "    %s: %s" % (key, value)
    except KeyError:
        print "    not found; please check lofar-shm config file."
        sys.exit(EXIT_ERROR)

    lofar.shm.config.set("job_control.rlimits", rlimits)


def main():
    global db, jobctrl

    # parse options from the command line.
    parser = optparse.OptionParser(usage="%prog [options]\nforemand.py is part of lofar-shm and is responsible for job control on shm client nodes.")
    parser.add_option("-d", "--daemonize", action="store_true", dest="daemonize", default=False, help="foremand will run as a daemon (default: false)")
    parser.add_option("-p", "--pid-path", action="store", dest="pid_path", default="/var/run", help="where to write pid file on start-up (default: /var/run).")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="write debug information to syslog (default: false)")
    (options,args) = parser.parse_args()

    print "checking configuration:"
    check_config()
    print "done."
    print

    # open syslog
    # maybe need syslog.LOG_NOWAIT when using SIGCHLD;
    # see p. 430 of Advanced Programming in the UNIX Environment
    syslog.openlog("foremand", syslog.LOG_PID, syslog.LOG_DAEMON)
    if options.verbose:
        syslog.setlogmask(syslog.LOG_UPTO(syslog.LOG_DEBUG))
    else:
        syslog.setlogmask(syslog.LOG_UPTO(syslog.LOG_NOTICE))

    # daemonize if required
    if options.daemonize:
        daemonize()

    # ensure there is only one instance running
    try:
        pid_file = os.open(os.path.join(options.pid_path, "foremand.pid"), os.O_WRONLY|os.O_CREAT)
        fcntl.lockf(pid_file, fcntl.LOCK_EX|fcntl.LOCK_NB)
        os.ftruncate(pid_file, 0)
        os.write(pid_file, "%i\n" % os.getpid())
    except (IOError, OSError), ex:
        if options.daemonize:
            syslog.syslog(syslog.LOG_ERR, "another instance of foremand seems to be running (could not create and/or lock pid file in %s); daemon not started (%s)" % (options.pid_path, ex))
        else:
            print "error: another instance of foremand seems to be running (could not create and/or lock pid file in %s); daemon not started (%s)" % (options.pid_path, ex)

        sys.exit(EXIT_ERROR)

    # open a connection to the shm database
    db = lofar.shm.db.SysHealthDatabase()
    try:
        db.open()
    except lofar.shm.DatabaseError, ex:
        syslog.syslog(syslog.LOG_ERR, "%s():: could not connect to the database (%s)" % (sys._getframe().f_code.co_name, ex))
        sys.exit(EXIT_ERROR)

    # initialize job control
    jobctrl = lofar.shm.job_control.JobControl(db)

    # install SIGTERM handler
    if signal.getsignal(signal.SIGTERM) != signal.SIG_IGN:
        signal.signal(signal.SIGTERM, handle_quit)

    if not options.daemonize:
        # install SIGINT/SIGQUIT handlers
        if signal.getsignal(signal.SIGINT) != signal.SIG_IGN:
            signal.signal(signal.SIGINT, handle_quit)

        if signal.getsignal(signal.SIGQUIT) != signal.SIG_IGN:
            signal.signal(signal.SIGQUIT, handle_quit)

    cycle_time = lofar.shm.config.get("job_control.cycle_time")
    syslog.syslog(syslog.LOG_DEBUG, "%s():: setting cycle time to %i second(s)" % (sys._getframe().f_code.co_name, cycle_time))

    syslog.syslog(syslog.LOG_NOTICE, "%s():: up and running" % sys._getframe().f_code.co_name)
    while not quit:
        cycle()
        time.sleep(cycle_time)

    syslog.syslog(syslog.LOG_NOTICE, "%s():: shutting down..."  % sys._getframe().f_code.co_name)

    # update processTable (important for logging)
    handle_exited_jobs()

    # SIGKILL any remaining jobs
    if len(processTable) > 0:
        syslog.syslog(syslog.LOG_NOTICE, "%s():: sending SIGKILL to %i remaining job(s)"  % (sys._getframe().f_code.co_name, len(processTable)))

        send_signal(processTable, signal.SIGKILL)
        time.sleep(3)

        # update processTable (important for logging)
        handle_exited_jobs()

    # close connection to the database
    try:
        db.close()
    except lofar.shm.DatabaseError, ex:
        syslog.syslog(syslog.LOG_ERR, "%s():: unable to close the database (%s)"  % (sys._getframe().f_code.co_name, ex))

    # close syslog
    syslog.syslog(syslog.LOG_NOTICE, "%s():: shut down sequence complete, good bye"  % sys._getframe().f_code.co_name)
    syslog.closelog()

    sys.exit(0)

if __name__ == "__main__":
    main()
