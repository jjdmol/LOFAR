import os
import sys
import logging

# log levels (from /usr/include/sys/syslog.h)
LOG_EMERG   = 0   # system is unusable
LOG_ALERT   = 1   # action must be taken immediately
LOG_CRIT    = 2   # critical conditions
LOG_ERR     = 3   # error conditions
LOG_WARNING = 4   # warning conditions
LOG_NOTICE  = 5   # normal but significant condition
LOG_INFO    = 6   # informational
LOG_DEBUG   = 7   # debug-level messages


class Job(object):
    def __init__(self, id, ticket_no, command=None):
        self.id = id
        self.ticket_no = ticket_no
        self.command = command

# The JobLogHandler logs to the SHM database

class JobLogHandler(logging.Handler):
    levelMap = { "CRITICAL": LOG_CRIT,
                 "ERROR"   : LOG_ERR,
                 "WARNING" : LOG_WARNING,
                 "INFO"    : LOG_INFO,
                 "DEBUG"   : LOG_DEBUG }

    def __init__(self, db, job=None):
        logging.Handler.__init__(self)
        
        self._db = db
        self._job = job

    def emit(self, record):
        if self._job is None:
            id = None
            ticket_no = None
        else:
            id = self._job.id
            ticket_no = self._job.ticket_no
        
        try:
            level = self.levelMap[record.levelname]
        except KeyError:
            level = record.levelno
        
        self._db.perform_query("""
                        SELECT job_control.log(%s, %s, %s, %s, %s, %s, %s, inet_client_addr());
                        """, (level, record.process, record.pathname, -1, record.getMessage(), id, ticket_no))
    
# The JobControl class provides wrappers to the stored procedures residing in the SHM database.

class JobControl(object):
    def __init__(self, db):
        self._db = db

    def log(self, job, level, message):
        frame = sys._getframe(1)
            
        if job is None:
            self._db.perform_query("""
                            SELECT job_control.log(%s, %s, %s, %s, %s, NULL, NULL, inet_client_addr());
                            """, (level, os.getpid(), frame.f_code.co_filename, frame.f_lineno, message))
        else:
            self._db.perform_query("""
                            SELECT job_control.log(%s, %s, %s, %s, %s, %s, %s, inet_client_addr());
                            """, (level, os.getpid(), frame.f_code.co_filename, frame.f_lineno, message, job.id, job.ticket_no))
    
    def get_info(self, job):
        try:
            [row] = self._db.perform_query("SELECT * FROM job_control.queue WHERE id = %s", (job.id,))
            job.command = row.command
        except (TypeError, ValueError):
            pass
        
    def update_queue(self):
        self._db.perform_query("SELECT job_control.update_queue();")
   
    def bind(self):
        try:
            [row] = self._db.perform_query("SELECT * FROM job_control.bind() AS (id bigint, ticket_no bigint);")
            if (row.id is None) or (row.ticket_no is None):
                return None
            else:
                return Job(row.id, row.ticket_no)
        except (TypeError, ValueError):
            return None
    
    def unbind(self, job):
        try:
            [row] = self._db.perform_query("SELECT job_control.unbind(%s, %s) AS result;", (job.id, job.ticket_no))
            return bool(row.result)
        except (TypeError, ValueError):
            return False
    
    def reset_watchdog(self, job):
        try:
            [row] = self._db.perform_query("SELECT job_control.reset_watchdog(%s, %s) AS result;", (job.id, job.ticket_no))
            return bool(row.result)
        except (TypeError, ValueError):
            return False
    
    def start(self, id):
        try:
            [row] = self._db.perform_query("SELECT job_control.start(%s) AS result;", (id,))
            return bool(row.result)
        except (TypeError, ValueError):
            return False
        
    def stop(self, id):
        try:
            [row] = self._db.perform_query("SELECT job_control.stop(%s) AS result;", (id,))
            return bool(row.result)
        except (TypeError, ValueError):
            return False
    
    def valid(self, job):
        try:
            [row] = self._db.perform_query("SELECT job_control.valid(%s, %s) AS result;", (job.id, job.ticket_no))
            return bool(row.result)
        except (TypeError, ValueError):
            return False

    def conditional_commit(self, job):
        self._db.perform_query("SELECT job_control.conditional_commit(%s, %s); COMMIT;", (job.id, job.ticket_no))
