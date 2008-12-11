from django.template.loader import get_template
from django.shortcuts import render_to_response
from django.http import HttpResponse
from django.template import RequestContext
import datetime
from  diagnoses import *
import plots

def entrance(request):

    #Get some diagnoses for default view
    sites = ['cs001','cs008','cs010','cs016']
    site_diagnoses = [(diagnose_site(s)) for s in sites]
   #for rendering, reorder
    site_diag_table = []
    for site in sites:
        site_diag_table.append([site,diagnose_site(site)])

    #for footer
    now = datetime.datetime.now()
    ipaddr = request.META['REMOTE_ADDR']

    site_nums  = {'cs001':1001,'cs008':1008,'cs010':1010,'cs016':1016}
    
    #render
    return render_to_response('SHM_base.html',
                              {'diagnose_LOFAR':diagnose_LOFAR(),
                               'site_list':sites,
                               'site_diagnoses':site_diagnoses,
                               'site_diag_table':site_diag_table,
                               'timestamp':now,
                               'remote_addr':ipaddr,
                               },
                              context_instance=RequestContext(request))

def serve_image(request):
    response = plots.get_pic(request)
    return response
