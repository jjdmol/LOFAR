#!/usr/bin/env python
import urllib, urllib2, cookielib, os.path, ClientForm, socket
from job_parser import jobState2String

class client:
    """This is an HTTP client that knows how to use the Single Sign On of Mom2. 
    It is used instead of a SOAP client, because SOAPpy doesn't support
    form handling and cookies."""
    def __init__(self, loginUrl, url, logoutUrl, logger = None):
        self._loginurl  = loginUrl
        self._url       = url
        self._logouturl = logoutUrl
        self._headers   = {'User-agent' : 'Mozilla/4.0 (compatible; http_login.py)'}
        self.logger     = logger
        ## The following is a global setting!
        socket.setdefaulttimeout(3600)

    def _login(self):
        try:
            cookiejar = cookielib.CookieJar() ## Not using any of the more specialised versions as we do not want to save them.
            hch = urllib2.HTTPCookieProcessor(cookiejar)
            hrh = urllib2.HTTPRedirectHandler() ## we need a redirect handler to handle the funcky stuff MoM does
            ## this code is for debugging
            ## hh  = urllib2.HTTPHandler()
            ## hhs  = urllib2.HTTPSHandler()
            ## hh.set_http_debuglevel(1)
            ## hhs.set_http_debuglevel(1)
            self.opener = urllib2.build_opener(hrh, hch)
            ##self.opener = urllib2.build_opener(hh, hhs, hrh, hch)
        
            request  = urllib2.Request(self._loginurl, None, self._headers)
            response = self.opener.open(request)  ## get's us a JSESSIONID in a cookie, uses a redirect

            forms              = ClientForm.ParseResponse(response)
            if len(forms) == 0:
                raise Exception('Unable to parse MoM login form or form not available')
            form               = forms[0]
            form['j_username'] = self.username
            form['j_password'] = self.password
            
            request  = form.click()
            response = self.opener.open(request) ## get's us a JSESSIONIDSSO in a cookie, uses a redirect
        except Exception, e:
            raise Exception("Logging into MoM failed: " + str(e))

    def _setStatus(self, exportID, status, message = None):
        try:
            statusUrl = self._url + '?exportId=' + str(exportID) + '&status=' + str(status)
            if message:
              statusUrl += '&message=' + str(message)
            if self.logger:
              self.logger.debug("updating MoM: " + statusUrl)
            request  = urllib2.Request(statusUrl, None, self._headers)
            response = self.opener.open(request) ## We tell what we want
            reply = response.readlines()
            if reply == ['ok']:
                result = (0, 'http_login updated ' + str(exportID) + ' to ' + jobState2String(int(status)))
            else:    
                result = (1, 'http_login for ' + str(exportID) + ' failed on: ' + str(reply))
        except Exception, e:
            return Exception(2, 'http_login failed with exception: ' + str(e))
        else:
            return result

    def _getSIP(self, Type, ArchiveId, StorageTicket, FileName, URI, FileSize, MD5Checksum, Adler32Checksum):
        try:
            xmlcontent = """<?xml version="1.0" encoding="UTF-8"?>
            <lofar:%s archiveId="%s" xmlns:lofar="http://www.astron.nl/MoM2-Lofar">
                <locations>
                    <location>
                        <uri>lta://%s/%s/%s</uri>
                    </location>
                </locations>
                <storageTicket>%s</storageTicket>
                <fileSize>%s</fileSize>
                <checksums>
                    <checksum>
                        <algorithm>MD5</algorithm>
                        <value>%s</value>
                    </checksum>
                    <checksum>
                        <algorithm>Adler32</algorithm>
                        <value>%s</value>
                    </checksum>
                </checksums>
            </lofar:%s>""" % (Type, ArchiveId, StorageTicket, FileName, URI, StorageTicket, FileSize, MD5Checksum, Adler32Checksum, Type)
            
            data = urllib.urlencode({"command" : "get-sip-with-input", "xmlcontent" : xmlcontent})
            # Now get that file-like object again, remembering to mention the data.
            request  = urllib2.Request(self._url, data, self._headers)
            response = self.opener.open(request)
            result = response.read()
            response.close()
            return result
        except Exception, e:
            raise Exception("getting SIP from MoM failed: " + str(e))
        return ''

    def _logout(self):
        try:
            request  = urllib2.Request(self._logouturl, None, self._headers)
            response = self.opener.open(request) ## we get out again
        except Exception, e:
            raise Exception("Logging out of MoM failed: " + str(e))

    def setStatus(self, exportID, status):
        self._login()
        result = self._setStatus(exportID, status)
        self._logout()
        return result

    def getSIP(self, ArchiveId, StorageTicket, FileName, URI, FileSize, MD5Checksum, Adler32Checksum):
        self._login()
        #result = self._getSIP("uvDataProduct", ArchiveId, StorageTicket, FileName, URI, FileSize, MD5Checksum, Adler32Checksum)
        result = self._getSIP("DataProduct", ArchiveId, StorageTicket, FileName, URI, FileSize, MD5Checksum, Adler32Checksum)
        self._logout()
        return result

