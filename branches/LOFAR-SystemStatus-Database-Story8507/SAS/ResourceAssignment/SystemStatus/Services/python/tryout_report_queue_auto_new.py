#!/usr/bin/python
import os, MySQLdb, datetime, sys

import psycopg2 as pg
import psycopg2.extras as pgdefs

import time

## HACK if you need LTAIngest in your path
#sys.path.append(os.getcwd() + "/../LTAIngest")
#sys.path.append(os.getcwd() + "/../LTAIngest/SOAPpy-0.12.0")
#sys.path.append(os.getcwd() + "/../LTAIngest/fpconst-0.7.0")

PGHOST     = "mcu005.control.lofar"
PGDATABASE = "datamonitor"
PGUSER     = "peterzon"
PGPWD      = "welkom001"

pgdb = pg.connect("host=%s dbname=%s user=%s password=%s" %(PGHOST, PGDATABASE, PGUSER, PGPWD))
msdb = MySQLdb.connect(host="mysql1.control.lofar", user="momreadonly", passwd="daub673(ming", db="lofar_mom3")

def pgcurs():
    global pgdb
    cur=pgdb.cursor(cursor_factory = pgdefs.RealDictCursor)
    cur.execute("SET CLIENT_ENCODING TO 'LATIN1';")
    return cur

def mscurs():
    global msdb
    return msdb.cursor()

def make_report(location):
  queue = os.listdir(location)
  report = {}
  for q in queue:
    name  = q.split('_')
    job   = name[1]
    obsId = name[3]
    if not job in report:
      report[job] = {}
    if obsId in report[job]:
      report[job][obsId] += 1
    else:
      report[job][obsId] = 1
  return report,len(queue)


def get_mom_jobs():
  c = mscurs()
  c.execute('SELECT * from export_jobs_advanced2;')
  db_jobs = c.fetchall()
  mom_jobs = {}
  for m in db_jobs:
    id = m[1]
    exportname = m[2]
    user = m[3]
    starttime = m[4].isoformat() if m[4] else None
    status = m[6]
    updatetime = m[7].isoformat() if m[4] else None
    projectname = m[8]
    location = m[9]
    mom_jobs[id] = (id, starttime, updatetime, user, status, exportname, projectname, location)
  return mom_jobs

def get_jobs():
  c = pgcurs()
  c.execute("SELECT id,description FROM export_jobs;") #no status used at this time
  db_jobs = c.fetchall()
  print db_jobs
  jobs = {}
  for d in db_jobs:
    jobs[d['id']] = (d['id'], d['description']) #no status used at this time
  return jobs

def del_job(job):
  c = pgcurs()
  nr = c.execute("DELETE FROM export_jobs WHERE id=%i;" % job)
  print ("DELETE FROM export_jobs WHERE id=%i;" % job)
  nr = c.execute("DELETE FROM observations WHERE job_id=%i;" % job)
  print ("DELETE FROM observations WHERE job_id=%i;" % job)

def add_job(job, desc, started="",update="",user="", state="", name="", project="",location=""):
  c = pgcurs()
  print ("INSERT INTO export_jobs VALUES(%i,'%s','%s','%s','%s','%s','%s','%s','%s','unknown');" % (job, desc, started, update, user, state, name, project, location))
  nr = c.execute("INSERT INTO export_jobs VALUES(%i,'%s','%s','%s','%s','%s','%s','%s','%s','unknown');" % (job, desc, started, update, user, state, name, project, location))
  print c.statusmessage
  pgdb.commit()

def del_obs(job):
  c = pgcurs()
  #c = db.cursor()
  nr = c.execute("DELETE FROM observations WHERE job_id=%i;" % job)

def update_job(job, obs, nr, state=None):
  if not state:
    state = 'unknown'
  c = pgcurs()
  c.execute("DELETE FROM observations WHERE obsid='%s' and job_id=%i;" % (obs, job))
  c.execute("UPDATE export_jobs set state='%s' WHERE id=%i;" % (state, job))
  nr = c.execute("INSERT INTO observations VALUES('%s',%i,%i);" % (obs,job,nr))

def update_total(total):
  c = pgcurs()
  nr = c.execute("UPDATE archiving_queues SET length=%i WHERE name='main';" % total)


def job_desc(job):
  return "Job: %d Start: %s Last update: %s User: %s State: %s Name: %s Project: %s Location: %s" % job

def report(report):
  for r in report[0]:
    try:
      job  = mom_jobs.pop(int(r))
      desc = job_desc(job)
    except:
      job = (r,"unknown","unknown","unknown","unknown","unknown","unknown","unknown")
      desc = 'MoM Job %s not found' % r
    if not int(r) in db_jobs:
      add_job(int(r), desc, job[1], job[2], job[3], job[4], job[5], job[6], job[7])
    else:
      db_jobs.pop(int(r))
    del_obs(int(r)) ##bit of a hack to clear all observations for this job
    #   ObsId : #files
    for o in report[0][r]:
      update_job(int(r), o, report[0][r][o], job[4])
  for d in db_jobs:
    del_job(d)
  update_total(report[1])


# 'Active Queue:'

main_queue = make_report('/log/ingest/jobs')
mom_jobs = get_mom_jobs()
db_jobs = get_jobs()
report(main_queue)
pgdb.commit()
print time.asctime() + ' ' + str(len(main_queue[0])) + ' ' + str(main_queue[1])
