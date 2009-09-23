from django.template.loader import get_template
from django.shortcuts import render_to_response
from django.template import RequestContext
from django.http import Http404
from django.http import HttpResponse
import datetime
import cStringIO
import diagnoses

def index(request):
    """
    shm.index(request) the default webpage on entry to the shm website

    """
    return render_to_response('index_shm.html',
                              {'footer':  footer(request),
                               'leftnav': shm_table(request,'sites'),
                               'header':  header(request,'home'),
                               }
                              )

#class Header(self):
#    def __init__(self:
#        self.title = 'LOFAR System Health Monitor'
#        self.system = 'lofar'
#        self.diagnosis = diagnoses.diagnose_system("lofar","lofar")


def header(request, page_type='home'):
    
    if page_type == 'home':
        header_title = 'LOFAR System Health Monitor'
        system = 'LOFAR'
        diagnosis = diagnoses.diagnose_system("lofar","lofar")
    return render_to_response('header_shm.html',
                              {'header_title' : header_title,
                               'system' : system,
                               'diagnosis' : diagnosis,
                               }
                              )

def footer(request):
    now = datetime.datetime.now()
    ipaddr = request.META['REMOTE_ADDR']
    return render_to_response('footer_shm.html',
                              {'timestamp': now,
                               'remote_addr':ipaddr,
                               }
                              )
    
def shm_table(request, table_type='sites'):
    """
    shm_table(HttpRequest request, tab_type='sites')
    
    Create tables of various types based on tab_type string.
    """
    if table_type == 'sites':
        sites = ['cs001','cs008','cs010','cs016']
        #for rendering, reorder
        site_diag_table = []
        for site in sites:
            site_diag_table.append([site,diagnoses.diagnose_site(site)])

        col_headings = ["","site","diagnosis"]
        return render_to_response('table_shm.html',
                                  {'title':'SITES',
                                   'colheads':col_headings,
                                   'ncols':2,
                                   'nrows':len(site_diag_table),
                                   'data':site_diag_table,
                                   }
                                  )
    else:
        raise Http404
    


