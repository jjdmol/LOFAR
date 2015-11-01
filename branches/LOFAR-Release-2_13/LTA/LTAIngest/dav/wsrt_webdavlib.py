import davlib

class wsrt_webdavlib():
    def __init__(self, logger):
        ## No inputs and outputs
        self.server          = 'localhost'
        self.davdir          = ''
        self.session_cookies = None
        self.logger          = logger

        ## Help text
        self.helptext = """
        Library containing webdav communication functions."""

    ## separate module that shares the webdav interaction of several export modules
    # the contents of the config should probably become part of this class in the future...
    def getConfig(self, name=None): ## The format of .serverrc is the same as the .nftprc of the FTP module
        ## Find the config file    
        import os.path, re
        r_field = re.compile(r'(?s)([^\n:]+): (.*?)(?=\n[^ \t]|\Z)')
        cwd = os.path.realpath(os.path.dirname(__file__))  + '/'
        config  = cwd + '.serverrc'
        self.logger.info('using server cofiguration in: ' + config)
        if os.path.exists(config): 
            s = open(config).read()
        else: 
            raise Exception('Server config not found')
    
        ## Parse the config file
        conf = {}
        s = s.replace('\r\n', '\n')
        s = s.replace('\r', '\n')
        for item in s.split('\n\n'):
            meta = dict(r_field.findall(item.strip()))
            if meta.has_key('name'):
                fname = meta['name']
                del meta['name']
                conf[fname] = meta
            else:
              raise Exception('Server config must include a name')
    
        if name is not None: 
            return conf[name]
        else: return conf

    def login(self):
        import binascii
        meta        = self.getConfig(self.server)
        auth        = binascii.b2a_base64(meta['username'] + ':' + meta['password'])
        self.dav    = davlib.DAV(meta['host'], meta['port'], protocol='http')
        self.davdir = (meta['remotedir'])
        logindir    = self.davdir + '/' ## N.B. "Magic" extra '/' needed in some cases, or we get a 302
#        self.dav.set_debuglevel(1) # for testing
        self.dav.connect()
        response = self.dav.get(logindir, {'Authorization': 'Basic %s' % auth.strip()})
        self.session_cookies = response.getheader('Set-Cookie')
        self.logger.debug('Status = %s, Reason = %s, Version = %s' % (response.status, response.reason, response.version))
        if not (response.version >= 10):
            raise Exception('Unknown protocol version:' + str(response.version))
        if not (response.status == 200 and response.reason == 'OK'):
##              or response.status == 302 and response.reason == 'Moved Temporarily'):
            self.logger.error('Got unrecognised answer with Status = %s, Reason = %s' % (response.status, response.reason))
            raise Exception('Problem logging in to WebDAV default repository: ' + meta['host']+ ':' + str(meta['port']) + self.davdir)
        response.close()
        self.dav.close()

    def setserver(self, server):
        self.server          = server
        self.session_cookies = None     

    def get(self, davURL):
        if not self.session_cookies:
            self.login()
        self.dav.connect()
        response = self.dav.get(self.davdir + davURL, {'Cookie':self.session_cookies})
        self.logger.debug('Status = %s, Reason = %s, Version = %s' % (response.status, response.reason, response.version))
        if not (response.version >= 10):
            raise Exception('Unknown protocol version:' + str(response.version))
        if not (response.status == 404 or response.status == 200): ##404 == does not exist, 200 == exists
            raise Exception('Unknown status response:' + str(response.status) + ':' + str(response.reason))
        response.close()
        self.dav.close()
        return response.status
        
    def mkdir(self, davdir):
        if not self.session_cookies:
            self.login()
        self.dav.connect()
        response = self.dav.mkcol(self.davdir + davdir, {'Cookie':self.session_cookies})
        self.logger.debug('Status = %s, Reason = %s, Version = %s' % (response.status, response.reason, response.version))
        if not (response.version >= 10):
            raise Exception('Unknown protocol version:' + str(response.version))
        if not (response.status == 405 or response.status == 201): ##405 == exists, 201 == created
            raise Exception('Unknown status response:' + str(response.status) + ':' + str(response.reason))
        response.close()
        self.dav.close()
        return response.status

    def storbinary(self, targetfile, binfile):
        if not self.session_cookies:
            self.login()
        self.dav.connect()
        response = self.dav.put(self.davdir + targetfile, binfile, None, None, {'Cookie':self.session_cookies})
        self.logger.debug('Status = %s, Reason = %s, Version = %s' % (response.status, response.reason, response.version))
        if not (response.version >= 10):
            raise Exception("Unknown protocol version:" + str(response.version))
        if not (response.status == 200 or response.status == 201 or response.status == 204): ##200 == exists, 201 == created, 204 = no content
            raise Exception('Unexpected status response:' + str(response.status) + ':' + str(response.reason))
        response.close()
        self.dav.close()
        return response.status

    def getbinary(self, sourcefile, binfile):
        if not self.session_cookies:
            self.login()
        self.dav.connect()
        response = self.dav.get(self.davdir + sourcefile, {'Cookie':self.session_cookies})
        self.logger.debug('Status = %s, Reason = %s, Version = %s' % (response.status, response.reason, response.version))
        if not (response.version >= 10):
            raise Exception("Unknown protocol version:" + str(response.version))
        if not (response.status == 200 or response.status == 404): ##200 == exists, 404 == does not exist
            raise Exception('Unexpected status response:' + str(response.status) + ':' + str(response.reason))
        binfile.write(response.read())
        response.close()
        self.dav.close()
        return response.status

    def upload(self, target, davroot):
        """target is a (source/root dir, destination dir/filename) tuple,
        the file to be sent is in target[0] + '/' + target[1],
        davpath is the base path in webdav, where target[1] will be put,
        target[1] can contain directories, those will be created
        if necessary."""
        import os.path
        path, filename = os.path.split(target[1])
        path           = path
        if self.get(davroot + path) == 404:
            self.logger.info('Creating folder %s' % path)
            temppath = davroot
            self.mkdir(temppath)
            for folder in path.split('/'): 
                temppath += '/' + folder
                self.mkdir(temppath)
        binfile = open(target[0] + '/' + target[1], 'rb')
        self.logger.info('Storing %s' % target[0] + '/' + target[1])
        try:
            ## does this raise an exception if something's wrong ?
            result = self.storbinary(davroot + path + '/' + filename, binfile) 
        except Exception, e:
            self.logger.error('Unable to store file %s' % target[1])
            binfile.close()
            raise Exception('WebDAV transfer failed with error: ' + str(e))
        binfile.close()

    ## input looks something like this:
    ##<?xml version="1.0" encoding="UTF-8"?>
    ##<D:multistatus xmlns:D="DAV:">
    ##    <D:response>
    ##        <D:href>/repository/mom2/R06B/006/207939/inspection_files</D:href>
    ##        <D:propstat>
    ##            <D:prop>
    ##                <D:resourcetype>
    ##                    <D:collection />
    ##                </D:resourcetype>
    ##            </D:prop>
    ##            <D:status>HTTP/1.1 200 OK</D:status>
    ##        </D:propstat>
    ##    </D:response>
    ##</D:multistatus>
    def parse_propfind(self, response):
#        print response ## for testing
        from xml.dom import minidom, Node
        doc = minidom.parseString(response)
        files = []
        dirs  = []
        if doc.documentElement.nodeName == 'D:multistatus':
            for node in doc.documentElement.childNodes:
                if node.nodeName == 'D:response':
                    collection = False
                    name       = ''
                    status     = ''
                    for responsenode in node.childNodes:
                        if responsenode.nodeName == 'D:href':
                            name = responsenode.childNodes[0].nodeValue
                        elif responsenode.nodeName == 'D:propstat':
                            for propstatnode in responsenode.childNodes:
                                if propstatnode.nodeName == 'D:prop':
                                    for propnode in propstatnode.childNodes:
                                        if propnode.nodeName == 'D:resourcetype':
                                            for resourcetypenode in propnode.childNodes:
                                                if resourcetypenode.nodeName == 'D:collection':
                                                    collection = True
                                elif propstatnode.nodeName == 'D:status':
                                   status = propstatnode.childNodes[0].nodeValue
                    if name and collection and status == "HTTP/1.1 200 OK":
                        dirs.append(name.replace(self.davdir, '', 1))
## old repository                   elif name and status == "HTTP/1.1 404 Not Found": ## files do not have resourcetype
                    elif name and status == "HTTP/1.1 200 OK": ## files do not have resourcetype
                        files.append(name.replace(self.davdir, '', 1))
        return (files, dirs)
        
    def propfind(self, remotepath):
        """Actually only tries to find the resourcetype, not every property.
        As this will tell if its a directory. """
        if not self.session_cookies:
            self.login()
        self.dav.connect()
        response = self.dav.propfind(self.davdir + remotepath, '<propfind xmlns="DAV:"><prop><resourcetype/></prop></propfind>', 1, {'Cookie':self.session_cookies})
        if (response.status == 207): ## 207 is multistatus
            lines = response.read()
            response.close()
            self.dav.close()
            return lines
        else:
            self.dav.close()
            raise Exception("Propfind can't find the directory: " + remotepath)
        

    ## for example davroot can be /repository/mom2/R06B/006/207939/inspection_files
    def download(self, davroot, targetpath):
        """davroot is what to retrieve, targetpath is where to put it."""
        import os
        if self.get(davroot) == 404:
            self.logger.info('Folder does not exist %s' % davroot)
            return []
        dirs  = [davroot]
        files = []
        count = 0
        while dirs:
            for d in dirs:
                f, dirs = self.parse_propfind(self.propfind(d))
                files.extend(f)
## old repository               dirs.remove(d)
        
        for f in files:
            self.logger.debug('Retrieving file %s' % f)
            path, filename = os.path.split(f.replace(davroot, '', 1))
            if not os.path.exists(targetpath + path):
                os.makedirs(targetpath + path)
            binfile = open(targetpath + path + '/' + filename, 'wb+')
            if self.getbinary(f, binfile) == 404:
                self.logger.debug('file does not exist in repository: ' + str(f))
            binfile.close()

        return files
