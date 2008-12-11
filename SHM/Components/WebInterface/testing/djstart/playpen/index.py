from django.template.loader import get_template
from django.shortcuts import render_to_response
from django.http import HttpResponse
from django.template import RequestContext

sites = ["cs001","cs008","cs010","cs016"]
obsdatatypes = ["sst","xst","rsp","sky"]
job_path = '/home/lofartest/SHM/Components/JobScripts/'
site_info = {}

    
def sorry(request):

    response = HttpResponse(mimetype='text/plain')
    response.write("SHM webpages are temporarily unavailable.\n")
    response.write("Sorry, hoor.\n")
    return response

