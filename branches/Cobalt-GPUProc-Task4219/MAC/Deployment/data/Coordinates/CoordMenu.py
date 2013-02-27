#!/usr/bin/python
# P.Donker ASTRON

import sys,pgdb,pg
from subprocess import *
import os,sys,time
from database import *

# get info from database.py
dbName=getDBname()
dbHost=getDBhost()

VERSION = '0.0.1' # version of this script
default_targetdate='2009.5'

def menu():
    print """
    |=====================================|
    | Coordinates menu                    |
    |=====================================|
    | 0   do all (1,2,3,4,5,6,7,9,11)     |
    | 1   destroy and create CDB          |
    | 2   create CDB objects              |
    | 3   load all normal-vectors         |
    | 4   load all rotation matrices      |
    | 5   load all hba_rotations          |
    | 6   calculate all HBADeltas         |
    | 7   load all ETRF(expected) files   |
    | 8   load one measurement file       |
    | 9   transform all ETRF to ITRF      |
    | 10  transform one ETRF to ITRF      |
    | 11  make all conf files             |
    | 12  make one conf file              |
    | Q   quit                            |
    |_____________________________________|
    """

def getInputWithDefault(prompt, defaultValue):
    answer = defaultValue
    answer = raw_input(prompt+" ["+str(defaultValue)+"]: ")
    if (len(answer)==0): answer=defaultValue
    return answer

def create_CDB():
    print 'Creating new database'
    res = Popen('./create_CDB.sh').wait()
    print res
    
def create_CDB_objects():
    print 'Creating database objects'
    res = Popen('./create_CDB_objects.py').wait()
    print res

def load_normal_vectors():
    print 'Loading normal vectors'
    filename = getInputWithDefault("enter filename to load","data/normal_vectors.dat")
    if len(filename) == 0:
        print 'Error, No filename given'
        sys.exit()
    if not os.path.exists(filename):
        print "File does not exist"
        sys.exit()
    res = Popen(['./load_normal_vectors.py',filename]).wait()
    if (res != 0): sys.exit(1)
    #time.sleep(3)
    
def load_rotation_matrices():
    print 'Loading rotation matrices'
    filename = getInputWithDefault("enter filename to load","data/rotation_matrices.dat")
    if len(filename) == 0:
        print 'Error, No filename given'
        sys.exit()
    if not os.path.exists(filename):
        print "File does not exist"
        sys.exit()
    res = Popen(['./load_rotation_matrices.py',filename]).wait()
    if (res != 0): sys.exit(1)
    #time.sleep(3)
    
def load_hba_rotations():
    print 'Loading hba field rotations'
    filename = getInputWithDefault("enter filename to load","data/hba-rotations.csv")
    if len(filename) == 0:
        print 'Error, No filename given'
        sys.exit()
    if not os.path.exists(filename):
        print "File does not exist"
        sys.exit()
    res = Popen(['./load_hba_rotations.py',filename]).wait()
    if (res != 0): sys.exit(1)
    #time.sleep(3)

def calculate_hba_deltas():
    print 'calculating hba-deltas'
    #time.sleep(3)
    res = Popen(['./calc_hba_deltas.py']).wait()
    if (res != 0): sys.exit(1)
    
def load_all_ETRF():
    print 'loading all ETRF files from .//ETRF_FILES'
    os.chdir(os.curdir+'/ETRF_FILES')
    dirs = os.listdir(os.curdir)
    for dir in dirs:
        os.chdir(os.curdir+'/'+dir)
        files = os.listdir(os.curdir)
        for filename in files:
            if not os.path.exists(filename):
                print "File ",filename,"does not exist"
                sys.exit()
            res = Popen(['../../load_expected_pos.py',filename]).wait()
            if (res != 0): sys.exit(1)
        os.chdir(os.pardir)
    os.chdir(os.pardir)
    
def load_measurement():
    print 'load one measurement file'
    filename = getInputWithDefault("enter filename to load","")
    if len(filename) == 0:
        print 'Error, No filename given'
        sys.exit()
    if not os.path.exists(filename):
        print "File ",filename,"does not exist"
        sys.exit()
    res = Popen(['./load_measurementfile.py',filename]).wait()
    if (res != 0): sys.exit(1)

def transform_all():
    db = pg.connect(user="postgres", host=dbHost, dbname=dbName)
    print 'Transform all ETRF coordinates to ITRF coordinates for given date'
    target  = getInputWithDefault("Enter target_date",default_targetdate)
    all_stations=db.query("select distinct o.stationname from object o inner join field_rotations r on r.id = o.id").getresult();
    ref_stations=db.query("select distinct o.stationname from object o inner join reference_coord r on r.id = o.id").getresult();
    
    for stationname in ref_stations:
        station = stationname[0]
        if 0 != Popen(['./calc_coordinates.py',station,"LBA",target]).wait(): sys.exit(1)
        if 0 != Popen(['./calc_coordinates.py',station,"CLBA",target]).wait(): sys.exit(1)
        #if station[:1] == 'C': # core station
        if 0 != Popen(['./calc_coordinates.py',station,"HBA0",target]).wait(): sys.exit(1)
        if 0 != Popen(['./calc_coordinates.py',station,"CHBA0",target]).wait(): sys.exit(1)
        if 0 != Popen(['./calc_coordinates.py',station,"HBA1",target]).wait(): sys.exit(1)
        if 0 != Popen(['./calc_coordinates.py',station,"CHBA1",target]).wait(): sys.exit(1)
        #else: #remote or international station
        if 0 != Popen(['./calc_coordinates.py',station,"HBA",target]).wait(): sys.exit(1)
        if 0 != Popen(['./calc_coordinates.py',station,"CHBA",target]).wait(): sys.exit(1)
         
    db.close()
    missing_stations=list(set(all_stations) - set(ref_stations))
    for stationname in missing_stations:
        station = stationname[0]
        print "Station with known HBA rotation but no ETRF: ",station
    
    
def transform_one():
    print 'Transform ETRF coordinates to ITRF coordinates for given station and date'
    station = getInputWithDefault("Enter station       ","")
    anttype = getInputWithDefault("Enter type (LBA|HBA|HBA0|HBA1|CLBA|CHBA0|CHBA1|CHBA)","")
    target  = getInputWithDefault("Enter target_date   ",default_targetdate)
    res = Popen(['./calc_coordinates.py',station,anttype,target]).wait()
    if (res != 0): sys.exit(1)

def make_all_conf_files():
    db = pg.connect(user="postgres", host=dbHost, dbname=dbName)
    print 'Make all AntennaField.conf and iHBADeltas.conf files for given date'
    target  = getInputWithDefault("Enter target_date",default_targetdate)
    for stationname in db.query("select distinct o.stationname from object o inner join reference_coord r on r.id = o.id").getresult():
        station = stationname[0]
        res = Popen(['./make_conf_files.py',station,target]).wait()
        if (res != 0): sys.exit(1)
    res = Popen(['./make_all_station_file.py',target]).wait()
    if (res != 0): sys.exit(1)
    db.close()    
    
def make_one_conf_file():
    print 'Make one AntennaField.conf and iHBADeltas.conf file for given date'
    station = getInputWithDefault("Enter station    ","")
    target  = getInputWithDefault("Enter target_date",default_targetdate)
    res = Popen(['./make_conf_files.py',station,target]).wait()
    if (res != 0): sys.exit(1)


if __name__ == "__main__":
    while(1):
        menu()
        sel = raw_input('Enter choice :')
        if sel.upper() == 'Q': sys.exit(1)
        if sel == '1': create_CDB()
        if sel == '2': create_CDB_objects()
        if sel == '3': load_normal_vectors()
        if sel == '4': load_rotation_matrices()
        if sel == '5': load_hba_rotations()
        if sel == '6': calculate_hba_deltas()
        if sel == '7': load_all_ETRF()
        if sel == '8': load_measurement()
        if sel == '9': transform_all()
        if sel == '10': transform_one()
        if sel == '11': make_all_conf_files()
        if sel == '12': make_one_conf_files()
        if sel == '0':
            create_CDB()
            create_CDB_objects()
            load_normal_vectors()
            load_rotation_matrices()
            load_hba_rotations()
            calculate_hba_deltas()
            load_all_ETRF()
            transform_all()
            make_all_conf_files()
    

    
