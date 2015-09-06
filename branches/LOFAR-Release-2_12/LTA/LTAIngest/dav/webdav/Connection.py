# pylint: disable-msg=W0142,W0102,R0901,R0904,E0203,E1101,C0103
#
# Copyright 2008 German Aerospace Center (DLR)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


"""
The contained class extends the HTTPConnection class for WebDAV support.
"""


from httplib import HTTPConnection, CannotSendRequest, BadStatusLine, ResponseNotReady
from copy import copy
import base64   # for basic authentication
import md5
import mimetypes
import os       # file handling
import urllib
import types
import socket   # to "catch" socket.error
from threading import RLock
from davlib import DAV
from qp_xml import Parser

from webdav.WebdavResponse import MultiStatusResponse, ResponseFormatError
from webdav import Constants
from webdav.logger import getDefaultLogger


__version__ = "$LastChangedRevision$"


class Connection(DAV):
    """
    This class handles a connection to a WebDAV server.
    This class is used internally. Client code should prefer classes
    L{WebdavClient.ResourceStorer} and L{WebdavClient.CollectionStorer}.
    
    @author: Roland Betz
    """
    
    # Constants
    #  The following switch activates a workaround for the Tamino webdav server:
    #  Tamino expects URLs which are passed in a HTTP header to be Latin-1 encoded
    #  instead of Utf-8 encoded.
    #  Set this switch to zero in order to communicate with conformant servers.
    blockSize = 30000
    MaxRetries = 10
    
    def __init__(self, *args, **kwArgs):
        DAV.__init__(self, *args, **kwArgs)
        self.__authorizationInfo = None
        self.logger = getDefaultLogger()
        self.isConnectedToCatacomb = True
        self.serverTypeChecked = False
        self.lock = RLock()
         
    def _request(self, method, url, body=None, extra_hdrs={}):
        
        self.lock.acquire()
        try:
            # add the authorization header
            extraHeaders = copy(extra_hdrs)
            if self.__authorizationInfo:
                extraHeaders["AUTHORIZATION"] = self.__authorizationInfo.authorization
            
            # encode message parts
            body = _toUtf8(body)
            url = _urlEncode(url)
            for key, value in extraHeaders.items():
                extraHeaders[key] = _toUtf8(value)
                if key == "Destination": # copy/move header
                    if self.isConnectedToCatacomb:
                        extraHeaders[key] = _toUtf8(value.replace(Constants.SHARP, Constants.QUOTED_SHARP))
                        
                    else: # in case of TAMINO 4.4
                        extraHeaders[key] = _urlEncode(value)
            # pass message to httplib class
            for retry in range(0, Connection.MaxRetries):    # retry loop
                try:
                    self.logger.debug("REQUEST Send %s for %s" % (method, url))
                    self.logger.debug("REQUEST Body: " + repr(body))
                    for hdr in extraHeaders.items():
                        self.logger.debug("REQUEST Header: " + repr(hdr))
                    self.request(method, url, body, extraHeaders)
                    response = self.getresponse()
                    break  # no retry needed
                except (CannotSendRequest, socket.error, BadStatusLine, ResponseNotReady), exc:
                    # Workaround, start: reconnect and retry...
                    self.logger.debug("Exception: " + str(exc) + " Retry ... ")
                    self.close()
                    try:
                        self.connect()
                    except (CannotSendRequest, socket.error, BadStatusLine, ResponseNotReady), exc:
                        raise WebdavError("Cannot perform request. Connection failed.")
                    if retry == Connection.MaxRetries - 1:
                        raise WebdavError("Cannot perform request.")
            return self.__evaluateResponse(method, response)
        finally:
            self.lock.release()
        
    def __evaluateResponse(self, method, response):
        """ Evaluates the response of the WebDAV server. """
        
        status, reason = response.status, response.reason
        self.logger.debug("Method: " + method + " Status %d: " % status + reason)
        
        if status >= Constants.CODE_LOWEST_ERROR:     # error has occured ?
            self.logger.debug("ERROR Response: " + response.read())
            response.close()
            raise WebdavError(reason, status)
        
        if status == Constants.CODE_MULTISTATUS:
            content = response.read()
            ## check for UTF-8 encodig
            response.root = Parser().parse(content)
            try:
                response.msr = MultiStatusResponse(response.root)
            except ResponseFormatError:
                raise WebdavError("Invalid WebDAV response.")
            response.close()
            self.logger.debug("RESPONSE (Multi-Status): " + unicode(response.msr))
        elif method == 'LOCK' and status == Constants.CODE_SUCCEEDED:
            response.parse_lock_response()
            response.close()
        elif method != 'GET' and method != 'PUT':
            self.logger.debug("RESPONSE Body: " + response.read())
            response.close()
        return response
        
    def addBasicAuthorization(self, user, password, realm=None):
        if user and len(user) > 0:
            self.__authorizationInfo = _BasicAuthenticationInfo(realm=realm, user=user, password=password)
                   
    def addDigestAuthorization(self, user, password, realm=None):
        if user and len(user) > 0:
            self.__authorizationInfo = _DigestAuthenticationInfo(realm=realm, user=user, password=password)

    def putFile(self, path, srcfile, header={}):
        self.lock.acquire()
        try:
            # Assemble header
            size = os.fstat(srcfile.fileno()).st_size        
            header["Content-length"] = str(size)
            contentType, contentEnc = mimetypes.guess_type(path)
            if contentType:
                header['Content-Type'] = contentType
            if contentEnc:
                header['Content-Encoding'] = contentEnc
            if self.__authorizationInfo:
                header["AUTHORIZATION"] = self.__authorizationInfo.authorization
                
            # send first request
            path = _urlEncode(path)
            try:
                HTTPConnection.request(self, 'PUT', path, "", header)
                self._blockCopySocket(srcfile, self, Connection.blockSize)
                srcfile.close()
                response = self.getresponse()
            except (CannotSendRequest, socket.error, BadStatusLine, ResponseNotReady), exc:
                self.logger.debug("Exception: " + str(exc) + " Retry ... ")
                raise WebdavError("Cannot perform request.")
            status, reason = (response.status, response.reason)
            self.logger.debug("Status %d: %s" % (status, reason))
            try:
                if status >= Constants.CODE_LOWEST_ERROR:     # error has occured ?
                    raise WebdavError(reason, status)
            finally:
                self.logger.debug("RESPONSE Body: " + response.read())
                response.close()        
            return response
        finally:
            self.lock.release()
                  
    def _blockCopySocket(self, source, toSocket, blockSize):
        transferedBytes = 0
        block = source.read(blockSize)
        #while source.readinto(block, blockSize):
        while len(block):
            toSocket.send(block)
            self.logger.debug("Wrote %d bytes." % len(block))
            transferedBytes += len(block)
            block = source.read(blockSize)        
        self.logger.info("Transfered %d bytes." % transferedBytes)

    def __str__(self):
        return self.protocol + "://" + self.host + ':' + str(self.port)
        

class _BasicAuthenticationInfo(object):
    def __init__(self, **kwArgs):
        self.__dict__.update(kwArgs)
        self.cookie = base64.encodestring("%s:%s" % (self.user, self.password) ).strip()
        self.authorization = "Basic " + self.cookie
        self.password = None     # protect password security
        
class _DigestAuthenticationInfo(object):
    def __init__(self, **kwArgs):
        self.__dict__.update(kwArgs)
        value = "%s:%s:%s" % (self.user, self.realm, self.password)
        value = value.strip()
        self.extra = md5.new(value).digest()
        self.authorization = "Digest realm=%s,user=%s"


class WebdavError(IOError):
    def __init__(self, reason, code=0):
        IOError.__init__(self, code)
        self.code = code
        self.reason = reason
    def __str__(self):
        return self.reason

def _toUtf8(body):
    if body:
        if  type(body) != types.UnicodeType:
            body = unicode(body, 'latin-1')
        body = body.encode('utf-8')
    return body

def _toLatin1(body):
    if  type(body) == types.UnicodeType:    # unicode text detected
        body = body.encode('latin-1')
    return body

def _urlEncode(url):
    if  type(url) != types.UnicodeType:
        url = unicode(url, 'latin-1')
    if Constants.CONFIG_UNICODE_URL:
        url = url.encode('utf-8')
    return urllib.quote(url)
