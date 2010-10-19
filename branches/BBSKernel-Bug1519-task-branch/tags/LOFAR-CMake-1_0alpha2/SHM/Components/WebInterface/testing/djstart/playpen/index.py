from django.template.loader import get_template
from django.shortcuts import render_to_response
from django.http import HttpResponse
from django.template import RequestContext

sites = ["cs302","rs106","rs208","rs307","rs503"]
obsdatatypes = ["sst","xst","rsp","sky"]
job_path = '/home/avruch/work/lofar/shm/versions/Components/JobScripts/'
site_info = {}

def sorry(request):

    response = HttpResponse(mimetype='text/plain')
    response.write("SHM webpages are temporarily unavailable.\n")
    response.write("Sorry, hoor.\n")
    return response

