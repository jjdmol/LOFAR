from django.template.loader import get_template
from django.shortcuts import render_to_response
from django.http import HttpResponse, Http404
from django.template import RequestContext
from django import newforms as forms
import re
import subprocess
import lofar.shm.db
import display
import datetime
#import misc_info

sites = ["cs302","rs106","rs208","rs307","rs503"]

obsdatatypes = ["rsp","sst","xst","sky"]
job_path = '/home/avruch/work/lofar/shm/versions/Components/JobScripts/'
site_info = {}

class dateform(forms.Form):
    epoch = forms.DateTimeField(initial=datetime.datetime.utcnow(),
                                required=True,
                                help_text = " (format YYYY-MM-DD HH:MM:SS)")
class siteform(forms.Form):
    site = forms.ChoiceField(choices = ( ('cs302','cs302'),
                                         ('rs106','rs106'),
                                         ('rs208','rs208'),
                                         ('rs503','rs503'),
                                         ('rs307','rs307') ))
class datatypeform(forms.Form):
    datatype = forms.ChoiceField(choices = ( ('rsp','RSP board sensor values'),
                                             ('sst','Subband Statistics'),
                                             ('xst','Crosscorrelation Statistics'),
                                             ('sky','Sky Map') ) )
class ObservedData:
    def __init__(self,system='cs302',qtime=datetime.datetime.utcnow()):
        self.datasource = system
        self.date   =  qtime
        self.classification = []

class Stations:
    def __init__(self):
        self.names = []
        self.ids   = []
        self.ports = []
        self.inets = []
        self.number = 0

    def get_station_list(self):
        db = lofar.shm.db.SysHealthDatabase()
        db.open()

        query = "SELECT * FROM Lofar.MacInformationServers;"
        results = db.perform_query(query)
        db.close()
        count = 0
        for inst in results:
            if (inst.si_id > 1000) and (inst.si_id < 10000):
                self.names.append(inst.si_name.lower())
                self.ids.append(int(inst.si_id))
                self.ports.append(inst.mis_port)
                self.inets.append(inst.mis_address)
                count += 1
        return count
        
class XST(ObservedData):

    def __init__(self, system, qtime):
        ObservedData.__init__(self,system, qtime)
        self.namestring = "Cross Correlation Statistics"
        self.shortnamestring = "xst"
        #self.data = self.most_recent_observation()
        self.plot = ""
        #self.plot = display.xst(self.data)
        self.num_per_station = range(3)
        
    def most_recent_observation(thing,seqnum):
        db = lofar.shm.db.SysHealthDatabase()
        db.open()

        query = "SELECT si_id, subband, time, rcu_settings, classification, "\
                "geo_loc, ant_coord, acm_data FROM Lofar.AntennaCorrelationMatrices WHERE "\
                "(time <= %s) and " \
                "(si_id = %04d) ORDER BY time DESC LIMIT 3;"%\
                (thing.date.strftime("'%Y-%m-%d %H:%M:%S+00'"),display.name_to_si_id(thing.datasource))
        results = db.perform_query(query)
        db.close()
        if (len(results)>=seqnum+1):
            thing.valid = True
            return results[seqnum]
        else:
            thing.valid = False
            return None

    def fetch(thing):
        """fetch observation immediately from system,
        put into db, return stdout & stderr of fetch process """

        #kludge
        cmdname = "acm" + "%02d" % int(thing.datasource[-3:])
        
        db = lofar.shm.db.SysHealthDatabase()
        db.open()

        query = "SELECT name, command "\
                "FROM job_control.queue WHERE "\
                "(name = '%s');"%\
                (cmdname)
        results = db.perform_query(query)
        db.close()
        assert len(results) == 1

        cmd = job_path + results[0].command
        cmd = cmd.split()
        #ensure '-v' option, and remove any '-j' option
        new_cmd = []
        new_cmd.append(cmd[0])
        new_cmd.append('-v')
        for arg in cmd[1:]:
            if (arg != '-j'):
                new_cmd.append(arg)
        MyOut = subprocess.Popen(new_cmd,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT).communicate()
        return MyOut

class SKY(XST):
    def __init__(self, system, qtime):
        ObservedData.__init__(self,system, qtime)
        self.namestring = "Sky Map"
        self.shortnamestring = "sky"
        #self.data = self.most_recent_observation()
        self.num_per_station = range(3)
        
class SST(ObservedData):

    def __init__(self, system, qtime):
        ObservedData.__init__(self,system,qtime)
        self.namestring = "Subband Statistics"
        self.shortnamestring = "sst"
        #if (self.datasource != 'cs010'):
        #    self.num_per_station = range(32)
        #else:
        #    self.num_per_station = range(96)
        self.num_per_station = range(96)

        #self.numitems = {}
        #for site in sites:
        #    self.numitems[site] = 32
        #self.numitems['cs010'] = 96

    def most_recent_observation(thing,rcu_id=0):
        db = lofar.shm.db.SysHealthDatabase()
        db.open()
        query = "SELECT si_id, rcu_id, time, rcu_settings, classification,"\
                "spectrum FROM Lofar.SubbandStatistics WHERE "\
                "(time <= %s) and (rcu_id = %d) and"\
                "(si_id = %04d) ORDER BY time DESC LIMIT 1;"%\
                (thing.date.strftime("'%Y-%m-%d %H:%M:%S+00'"),
                 rcu_id,
                 display.name_to_si_id(thing.datasource))
        results = db.perform_query(query)
        db.close()
        if (len(results) > 0):
            return results[0]
        else:
            return None

#    def make_plot(dbinst,thumb=True):
#        if (dbinst == None):
#            return None
#        else:
#            return display.sst(dbinst,thumb)

#    def fill_plots(thing,thumb=True):
#        thing.plots = []
#        for rcu in thing.num_per_station:
#            plot = make_plot
#            if (plot != None):
#                thing.data.append(plot)
#        self.most_recent_observation()
#        #self.plot = display.sst(self.data[plotnumber])

    def fetch(thing):
        """fetch observation immediately from system,
        put into db, return stdout & stderr of fetch process """

        #kludge
        cmdname = "sb" + "%02d" % int(thing.datasource[-3:])
        
        db = lofar.shm.db.SysHealthDatabase()
        db.open()

        query = "SELECT name, command "\
                "FROM job_control.queue WHERE "\
                "(name = '%s');"%\
                (cmdname)
        results = db.perform_query(query)
        db.close()
        assert len(results) == 1

        cmd = job_path + results[0].command
        cmd = cmd.split()
        #ensure '-v' option, and remove any '-j' option
        new_cmd = []
        new_cmd.append(cmd[0])
        new_cmd.append('-v')
        for arg in cmd[1:]:
            if (arg != '-j'):
                new_cmd.append(arg)
        MyOut = subprocess.Popen(new_cmd,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT).communicate()
        return MyOut
    
class RSP(ObservedData):

    def __init__(self, system, qtime):
        ObservedData.__init__(self,system, qtime)
        self.namestring = "RSP Board Registers"
        self.shortnamestring = "rsp"
        #if (self.datasource != 'cs010'):
        #    self.num_per_station = range(4)
        #else:
        self.num_per_station = range(12)
        self.num_display = 1
        #self.data = self.most_recent_observation()
        self.plot = ""
        #self.plot = display.rsp(self.data[plotnumber])
        
    def most_recent_observation(thing,rsp_id=0):
        db = lofar.shm.db.SysHealthDatabase()
        db.open()

        query = "SELECT * "\
                "FROM Lofar.rspstatus WHERE "\
                "(time <= %s) and (rsp_id = %d) and "\
                "(si_id = %04d) ORDER BY time DESC LIMIT 1;"%\
                (thing.date.strftime("'%Y-%m-%d %H:%M:%S+00'"),
                 rsp_id,
                 display.name_to_si_id(thing.datasource))
        results = db.perform_query(query)
        db.close()
        if  (len(results) > 0):
            return results[0]
        else:
            return None

    def fetch(thing):
        """fetch observation immediately from system,
        put into db, return stdout & stderr of fetch process """

        #kludge
        cmdname = "rsp" + "%02d" % int(thing.datasource[-3:])
        
        db = lofar.shm.db.SysHealthDatabase()
        db.open()

        query = "SELECT name, command "\
                "FROM job_control.queue WHERE "\
                "(name = '%s');"%\
                (cmdname)
        results = db.perform_query(query)
        db.close()
        assert len(results) == 1

        cmd = job_path + results[0].command
        cmd = cmd.split()
        #ensure '-v' option, and remove any '-j' option
        new_cmd = []
        new_cmd.append(cmd[0])
        new_cmd.append('-v')
        for arg in cmd[1:]:
            if (arg != '-j'):
                new_cmd.append(arg)
        MyOut = subprocess.Popen(new_cmd,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT).communicate()
        return MyOut
    
def xst(request,station='cs302',plotnumber=0):

    if (request.method == 'POST'):
        view_date = dateform(request.POST)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()
        d = XST(station,query_date)

    elif (request.method == 'GET'):
        # kludge for Navigator web launch
        R = request.GET.copy()
        if (R.has_key("epoch") and R["epoch"].endswith('.html')):
            R["epoch"] = R["epoch"][:-5]
        view_date = dateform(R)
        # view_date = dateform(request.GET)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()

        d = XST(station,query_date)
    else:
        d = XST(station,datetime.datetime.utcnow())

    #testing
    #testsites = sites + [("foo") for i in range(92)]
    #pat = re.compile(".+image.+")

    im = re.compile(".+image.+")
    th = re.compile(".+thumb+.")
    fetch = re.compile(".+fetch.+")

    if fetch.match(request.META['PATH_INFO']):
        response = HttpResponse(mimetype='text/plain')
        response.write("daemon response: \n\n")
        response.write(d.fetch()[0])
        return response

    if im.match(request.META['PATH_INFO']):
        response = HttpResponse(mimetype='image/png')
        if th.match(request.META['PATH_INFO']):
            response.write(display.xst(d.most_recent_observation(int(plotnumber)), thumb=True))
        else:
            response.write(display.xst(d.most_recent_observation(int(plotnumber)), thumb=False))
        return response

    dbres = []
    classifs = {}
    for seqnr in d.num_per_station:
        foo = d.most_recent_observation(seqnr)
        if (foo != None):
            dbres.append(foo)
    if len(dbres) == 0 :
        classifs = {'noresults':1}
    else:
        classifs = {'nominal':[], 'offnominal':[]}
        #for dbinst in dbres:
        #    if dbinst != None:
        #        if ( (len(dbinst.classification) != 0) and (dbinst.classification[0] != 'NOMINAL')):
        #            classifs['offnominal'].append(seqnr)
        #        else:
        #            classifs['nominal'].append(seqnr)
 
    siites = Stations()
    siites.get_station_list()
    return render_to_response('data_shm.html',
                              {'data'  : d,
                               'classifs' : classifs,
                               'types' : obsdatatypes,
                               'sites' : siites.names,
                               'ipaddr': request.META['REMOTE_ADDR'],
                               })
        
def sst_plot(request,station='cs302',plotnumber=0):
    if (request.method == 'GET'):
        view_date = dateform(request.GET)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()

    d = SST(station,query_date)

    th = re.compile(".+thumb+.")

    response = HttpResponse(mimetype='image/png')
    if th.match(request.META['PATH_INFO']):
        # response.write(display.sst(d.data[int(plotnumber)], thumb=True))
        response.write(display.sst(d.most_recent_observation(int(plotnumber)), thumb=True))
    else:
        response.write(display.sst(d.most_recent_observation(int(plotnumber)), thumb=False))
        # response.write(d.plot)
    return response


def sst(request,station='cs302',plotnumber=0):

    if (request.method == 'POST'):
        view_date = dateform(request.POST)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()
        d = SST(station,query_date)

    elif (request.method == 'GET'):
        # kludge for Navigator web launch
        R = request.GET.copy()
        if (R.has_key("epoch") and R["epoch"].endswith('.html')):
            R["epoch"] = R["epoch"][:-5]
        view_date = dateform(R)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()

        d = SST(station,query_date)
    else:
        d = SST(station,datetime.datetime.utcnow())

    im = re.compile(".+image.+")
    th = re.compile(".+thumb+.")
    fetch = re.compile(".+fetch.+")

    if fetch.match(request.META['PATH_INFO']):
        response = HttpResponse(mimetype='text/plain')
        response.write("daemon response: \n\n")
        response.write(d.fetch()[0])
        return response

    if im.match(request.META['PATH_INFO']):
        response = HttpResponse(mimetype='image/png')
        if th.match(request.META['PATH_INFO']):
            # response.write(display.sst(d.data[int(plotnumber)], thumb=True))
            response.write(display.sst(d.most_recent_observation(int(plotnumber)), thumb=True))
        else:
            response.write(display.sst(d.most_recent_observation(int(plotnumber)), thumb=False))
        #response.write(d.plot)
        return response

    # prepare for thumbnail display by counting the types of classifications coming.
    # This fcn will be immediately called again to display the sst thumbnails, and I
    # want to arrange them nicely

    dbres = []
    classifs = {}
    for rcu_id in d.num_per_station:
        foo = d.most_recent_observation(rcu_id)
        if (foo != None):
            dbres.append(foo)
    if len(dbres) == 0 :
        classifs = {'noresults':1}
    else:
        classifs = {'nominal':[], 'offnominal':[]}
        for dbinst in dbres:
            if dbinst != None:
                if (type(dbinst.classification) == type([])):
                    if ((dbinst.classification[0] == 'NOMINAL') or 
                        (dbinst.classification[0] == 'ANTENNA_OFF')):
                        classifs['nominal'].append(dbinst.rcu_id)
                    else:
                        classifs['offnominal'].append(dbinst.rcu_id)
                else:
                    if (dbinst.classification != 'SKY'):
                        classifs['offnominal'].append(dbinst.rcu_id)
                    else:
                        classifs['nominal'].append(dbinst.rcu_id)
    #assert False
    siites = Stations()
    siites.get_station_list()
    return render_to_response('data_shm.html',
                              {'data'     : d,
                               'classifs' : classifs,
                               'types'    : obsdatatypes,
                               'sites'    : siites.names,
                               'ipaddr'   : request.META['REMOTE_ADDR'],
                               })
    
def rsp(request,station='cs302',plotnumber=0):

    if (request.method == 'POST'):
        view_date = dateform(request.POST)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()
        d = RSP(station,query_date)

    elif (request.method == 'GET'):
        # kludge for Navigator web launch
        R = request.GET.copy()
        if (R.has_key("epoch") and R["epoch"].endswith('.html')):
            R["epoch"] = R["epoch"][:-5]
        view_date = dateform(R)
        # view_date = dateform(request.GET)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()

        d = RSP(station,query_date)
    else:
        d = RSP(station,datetime.datetime.utcnow())

    pat = re.compile(".+image.+")
    fetch = re.compile(".+fetch.+")

    if fetch.match(request.META['PATH_INFO']):
        response = HttpResponse(mimetype='text/plain')
        response.write("daemon response: \n\n")
        response.write(d.fetch()[0])
        return response

    if pat.match(request.META['PATH_INFO']):
        response = HttpResponse(mimetype='text/plain')
        response.write(display.rsp(d.most_recent_observation(int(plotnumber))))
        #response.write(d.plot)
        #assert False
        return response

    dbres = []
    classifs = {}
    for rsp_id in d.num_per_station:
        foo = d.most_recent_observation(rsp_id)
        if (foo != None):
            dbres.append(foo)
    if len(dbres) == 0 :
        classifs = {'noresults':1}
    else:
        classifs = {'nominal':[], 'offnominal':[]}
        for dbinst in dbres:
            if dbinst != None:
                #if ( (len(dbinst.classification) != 0) and (dbinst.classification[0] != 'NOMINAL')):
                if ((len(dbinst.classification) == 0) or (dbinst.classification[0] == 'NOMINAL')):
                    classifs['nominal'].append(dbinst.rsp_id)
                else:
                    classifs['offnominal'].append(dbinst.rsp_id)

    siites = Stations()
    siites.get_station_list()
    return render_to_response('data_shm.html',
                              {'query_time' : view_date,
                               'data'  : d,
                               'classifs' : classifs,
                               'types' : obsdatatypes,
                               'sites' : siites.names,
                               'ipaddr': request.META['REMOTE_ADDR'],
                               })
def sky(request,station='cs302',plotnumber=0):

    if (request.method == 'POST'):
        view_date = dateform(request.POST)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()
        d = SKY(station,query_date)

    elif (request.method == 'GET'):
        view_date = dateform(request.GET)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()

        d = SKY(station,query_date)
    else:
        d = SKY(station,datetime.datetime.utcnow())

    im = re.compile(".+image.+")
    th = re.compile(".+thumb+.")
    fetch = re.compile(".+fetch.+")

    if fetch.match(request.META['PATH_INFO']):
        response = HttpResponse(mimetype='text/plain')
        response.write("daemon response: \n\n")
        response.write(d.fetch()[0])
        return response

    if im.match(request.META['PATH_INFO']):
        response = HttpResponse(mimetype='image/png')
        if th.match(request.META['PATH_INFO']):
            response.write(display.sky(d.most_recent_observation(int(plotnumber)), thumb=True))
        else:
            response.write(display.sky(d.most_recent_observation(int(plotnumber)), thumb=False))
        return response

    dbres = []
    classifs = {}
    for seqnr in d.num_per_station:
        foo = d.most_recent_observation(seqnr)
        if (foo != None):
            dbres.append(foo)
    if len(dbres) == 0 :
        classifs = {'noresults':1}
    else:
        classifs = {'nominal':[], 'offnominal':[]}
        #for dbinst in dbres:
        #    if dbinst != None:
        #        if ( (len(dbinst.classification) != 0) and (dbinst.classification[0] != 'NOMINAL')):
        #            classifs['offnominal'].append(seqnr)
        #        else:
        #            classifs['nominal'].append(seqnr)
 
    siites = Stations()
    siites.get_station_list()
    return render_to_response('data_shm.html',
                              {'data'  : d,
                               'classifs' : classifs,
                               'types' : obsdatatypes,
                               'sites' : siites.names,
                               'ipaddr': request.META['REMOTE_ADDR'],
                               })
        
def defaultpage(request):

    if (request.method == 'POST'):
        view_date = dateform(request.POST)
        if (view_date.is_valid()):
            query_date = view_date.clean_data['epoch']
        else:
            query_date = datetime.datetime.utcnow()
        d = SKY('cs302',query_date)
    else:
        d = SKY('cs302',datetime.datetime.utcnow())

    dbres = []
    classifs = {}
    for seqnr in d.num_per_station:
        foo = d.most_recent_observation(seqnr)
        if (foo != None):
            dbres.append(foo)
    if len(dbres) == 0 :
        classifs = {'noresults':1}
    else:
        classifs = {'nominal':[], 'offnominal':[]}
    
    siites = Stations()
    siites.get_station_list()
    return render_to_response('data_shm_default.html',
                              {'data'    : d,
                               'classifs': classifs,
                               'types'   : obsdatatypes,
                               'sites'   : siites.names,
                               'ipaddr'  : request.META['REMOTE_ADDR'],
                               'reload_url' :  '/shm/data/',
                               })
