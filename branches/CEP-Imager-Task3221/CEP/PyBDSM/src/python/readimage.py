"""Module readimage.

Defines operation Op_readimage which loads image from FITS file or uses Pyrap

The current implementation tries to reduce input file to 2D if
possible, as this makes more sence atm. One more important thing
to note -- in it's default configuration pyfits will read data
in non-native format, so we have to convert it before usage. See
the read_image_from_file in functions.py for details.

Lastly, wcs and spectal information are stored in img.wcs_obj and
img.freq_pars to remove any FITS-specific calls to the header,
etc. in other modules.
"""

import numpy as N
from image import *
from functions import read_image_from_file
import mylogger
import sys

Image.imagename = String(doc="Identifier name for output files")
Image.filename = String(doc="Name of input file without FITS extension")
Image.bbspatchnum = Int(doc="To keep track of patch number for bbs file "\
                            "for seperate patches per source")
Image.cfreq = Float(doc="Frequency in the header")
Image.use_io = String(doc="pyfits or pyrap")
Image.j = Int(doc="Wavelet order j, 0 for normal run")
Image.freq_pars = Tuple((0.0, 0.0, 0.0),
                        doc="Frequency prarmeters from the header: (crval, cdelt, crpix)")
Image.waveletimage = Bool(doc="Whether a wavelet transform image of not")
Image.pixel_beamarea = Float(doc="Beam area in pixel")
Image.equinox = Float(2000.0, doc='Equinox of input image from header')

class Op_readimage(Op):
    """Image file loader

    Loads fits file 'opts.fits_name' and configures
    wcslib machinery for it.
    """
    def __call__(self, img):
        import time, os
        mylog = mylogger.logging.getLogger("PyBDSM."+img.log+"Readimage")

        # Check for trailing "/" in filename (happens a lot, since MS images are directories)
        # Although the general rule is to not alter the values in opts (only the
        # user should be able to alter these), in this case there is no harm in
        # replacing the filename in opts with the '/' trimmed off.
        if img.opts.filename == '':
            raise RuntimeError('Image file name not specified.')
        if img.opts.filename[-1] == '/':
            img.opts.filename = img.opts.filename[:-1]

        # Determine indir if not explicitly given by user (in img.opts.indir)
        if img.opts.indir == None:
            indir = os.path.dirname(img.opts.filename)
            if indir == '':
                indir = './'
            img.indir = indir
        else:
            img.indir = img.opts.indir
            
        image_file = os.path.basename(img.opts.filename)
        result = read_image_from_file(image_file, img, img.indir)
        if result == None:
            raise RuntimeError("Cannot open file " + repr(image_file) + ". " + img._reason)
        else:
            data, hdr = result

        # Store data and header in img. If polarisation_do = False, only store pol == 'I'
        mylogger.userinfo(mylog, 'Image size',
                          str(data.shape[-2:])+' pixels')
        mylogger.userinfo(mylog, 'Number of channels',
                          '%i' % data.shape[1])
        mylogger.userinfo(mylog, 'Number of Stokes parameters',
                          '%i' % data.shape[0])
        if img.opts.polarisation_do and data.shape[0] == 1:
            img.opts.polarisation_do = False
            mylog.warning('Image has Stokes I only. Polarisation module disabled.')
        if img.opts.polarisation_do or data.shape[0] == 1:
            img.image = data
        else:
            img.image = data[0,:].reshape(1, data.shape[1], data.shape[2], data.shape[3])
        img.header = hdr
        img.j = 0                    

        ### initialize wcs conversion routines
        self.init_wcs(img)
        self.init_beam(img)
        self.init_freq(img)
        year, code = self.get_equinox(img)
        if year == None:
            mylog.info('Equinox not found in image header. Assuming J2000.')
            img.equinox = 2000.0
        else:
            img.equinox = year

        # Try to trim common extensions from filename
        root, ext = os.path.splitext(img.opts.filename)
        if ext in ['.fits', '.FITS', '.image']:
            fname = root
        else:
            fname = img.opts.filename
        img.filename = img.opts.filename
        img.parentname = fname
        img.imagename = fname+'.pybdsm'
        img.waveletimage = False
        if img.opts.output_all:
            # Set up directory to write output to
            basedir = './'+fname+'_pybdsm'
            opdir = img.opts.opdir_overwrite
            if opdir not in ['overwrite', 'append']: 
                img.opts.opdir_overwrite = 'append'
                mylog.info('Appending output files in directory '+basedir)
            img.basedir = basedir + '/'
            if img.opts.solnname != None: img.basedir += img.opts.solnname + '__'
            img.basedir += time.strftime("%d%b%Y_%H.%M.%S")

            if os.path.isfile(basedir): os.system("rm -fr "+basedir)
            if not os.path.isdir(basedir): os.mkdir(basedir)
            if opdir == 'overwrite': os.system("rm -fr "+basedir+"/*")
            os.mkdir(img.basedir)

        # Check for zeros and blank if img.opts.blank_zeros is True
        if img.opts.blank_zeros:
            zero_pixels = N.where(img.image[0] == 0.0)
            img.image[0][zero_pixels] = N.nan
        
        img.completed_Ops.append('readimage')
        return img

    def init_wcs(self, img):
        """Initialize wcs pixel <=> sky conversion routines, and
        store them as img.pix2sky & img.sky2pix.

        Thanks to transpose operation done to image earlier we can use
        p2s & s2p transforms directly.
        
        Both WCSLIB (from LOFAR svn) and PyWCS (http://stsdas.stsci.edu/
        astrolib/pywcs/, available from https://trac6.assembla.com/astrolib) 
        are supported.
        """
        try:
            import wcslib
            img.use_wcs = 'wcslib'
        except ImportError:
            try:
                import pywcs    
                img.use_wcs = 'pywcs'
            except ImportError:
                raise RuntimeError('Either WCSLIB or PyWCS is required.')
        from math import pi

        hdr = img.header
        if img.use_wcs == 'wcslib':
            t = wcslib.wcs()
        elif img.use_wcs == 'pywcs':
            t = pywcs.WCS(naxis=2)   
             
        if img.use_io == 'fits':
          crval = [hdr['crval1'], hdr['crval2']]
          crpix = [hdr['crpix1'], hdr['crpix2']]
          cdelt = [hdr['cdelt1'], hdr['cdelt2']]
          acdelt = [abs(hdr['cdelt1']), abs(hdr['cdelt2'])]
          ctype = [hdr['ctype1'], hdr['ctype2']]
          if hdr.has_key('crota1'): 
            crota = [hdr['crota1'], hdr['crota2']]
          else:
            crota = []
          if hdr.has_key('cunit1'): 
            cunit = [hdr['cunit1'], hdr['cunit2']]
          else:
            cunit = []

        if img.use_io == 'rap':
          wcs_dict = hdr['coordinates']['direction0']
          coord_dict = {'degree' : 1.0, 'arcsec' : 1.0/3600, 'rad' : 180.0/pi}
          iterlist = range(2)
          co_conv = [coord_dict[wcs_dict['units'][i]] for i in iterlist]
          crval = [wcs_dict.get('crval')[i]*co_conv[i] for i in iterlist]
          crpix = [wcs_dict.get('crpix')[i] for i in iterlist]
          cdelt = [wcs_dict.get('cdelt')[i]*co_conv[i] for i in iterlist]
          acdelt = [abs(wcs_dict.get('cdelt')[i])*co_conv[i] for i in iterlist]
          ctype = ['RA---' + wcs_dict.get('projection'), 'DEC--' + wcs_dict.get('projection')]
          if wcs_dict.has_key('crota1'):
            crota = [wcs_dict.get('crota')[i] for i in iterlist]
          else:
            crota = []
          if wcs_dict.has_key('cunit1'): 
            cunit = [wcs_dict.get('cunit')[i] for i in iterlist]
          else:
            cunit = []

        # Check if user has specified a subimage. If so, adjust crpix
        if img.opts.trim_box != None:
            xmin, xmax, ymin, ymax = img.trim_box
            crpix[0] -= xmin
            crpix[1] -= ymin

        if img.use_wcs == 'wcslib':
            t.crval = tuple(crval)
            t.crpix = tuple(crpix)
            t.cdelt = tuple(cdelt)
            t.acdelt = tuple(acdelt)
            t.ctype = tuple(ctype)
            if crota != []:
                t.crota = tuple(crota)
            if cunit != []:
                t.cunit = tuple(cunit)
            t.wcsset()
            img.wcs_obj = t
            img.pix2sky = t.p2s
            img.sky2pix = t.s2p
        elif img.use_wcs == 'pywcs':
            # Here we define new p2s and s2p methods to match those of wcslib
            def p2s(self, xy):
                xy_arr = N.array([xy])
                sky = self.wcs_pix2sky(xy_arr, 0, ra_dec_order=True)
                return sky.tolist()[0]
            def s2p(self, rd):
                rd_arr = N.array([rd])
                pix = self.wcs_sky2pix(rd_arr, 0, ra_dec_order=True)
                return pix.tolist()[0]
            instancemethod = type(t.wcs_pix2sky)
            t.p2s = instancemethod(p2s, t, pywcs.WCS)
            instancemethod = type(t.wcs_sky2pix)
            t.s2p = instancemethod(s2p, t, pywcs.WCS)
          
            t.wcs.crval = crval
            t.wcs.crpix = crpix
            t.wcs.cdelt = cdelt
            t.wcs.ctype = ctype
            if crota != []:
                t.wcs.crota = crota
            if cunit != []:
                t.wcs.cunit = cunit

            img.wcs_obj = t
            img.wcs_obj.acdelt = acdelt
            img.wcs_obj.crval = crval
            img.wcs_obj.crpix = crpix
            img.wcs_obj.cdelt = cdelt
            img.wcs_obj.ctype = ctype
            if crota != []:
                img.wcs_obj.crota = crota
            if cunit != []:
                img.wcs_obj.cunit = cunit
            
            img.pix2sky = t.p2s
            img.sky2pix = t.s2p

    def init_beam(self, img):
        """Initialize beam parameters, and conversion routines
        to convert beam to/from pixel coordinates"""
        from const import fwsig
        mylog = mylogger.logging.getLogger("PyBDSM.InitBeam")

        ### FIXME: beam shape conversion should include rotation angle
        hdr = img.header
        cdelt1, cdelt2 = img.wcs_obj.acdelt[0:2]

        ### define beam conversion routines:
        def beam2pix(x):
            """ Converts beam in deg to pixels """
            bmaj, bmin, bpa = x
            s1 = abs(bmaj/cdelt1) 
            s2 = abs(bmin/cdelt2) 
            th = bpa ### FIXME: check conventions (th + 90)
            return (s1, s2, th)

        def pix2coord(pix):
            x, y = pix
            s1 = abs(x*cdelt1) 
            s2 = abs(y*cdelt2) 
            return (s1, s2)

        def pix2beam(x):
            s1, s2, th = x
            bmaj = abs(s1*cdelt1) 
            bmin = abs(s2*cdelt2) 
            bpa  = th ### FIXME: check conventions (th - 90)
            if bmaj < bmin:
                bmaj, bmin = bmin, bmaj
                bpa += 90
            bpa = divmod(bpa, 180)[1] ### bpa lies between 0 and 180
            return (bmaj, bmin, bpa)

        ### Get the beam information from the header
        found = False
        if img.opts.beam is not None:
            beam = img.opts.beam
        else:
            if img.use_io == 'rap':
                iminfo = hdr['imageinfo']
                if iminfo.has_key('restoringbeam'):
                    beaminfo = iminfo['restoringbeam']
                    if beaminfo.has_key('major') and beaminfo.has_key('minor') and beaminfo.has_key('major'):
                        bmaj = beaminfo['major']['value']
                        bmin = beaminfo['minor']['value']
                        bpa = beaminfo['positionangle']['value']
                        # make sure all values are in degrees
                        if beaminfo['major']['unit'] == 'arcsec':
                            bmaj = bmaj / 3600.0
                        if beaminfo['minor']['unit'] == 'arcsec':
                            bmin = bmin / 3600.0
                        if beaminfo['major']['unit'] == 'rad':
                            bmaj = bmaj * 180.0 / N.pi
                        if beaminfo['minor']['unit'] == 'rad':
                            bmin = bmin * 180.0 / N.pi
                        beam = (bmaj, bmin, bpa) # all degrees
                        found = True
            if img.use_io == 'fits':
                try:
                    beam = (hdr['BMAJ'], hdr['BMIN'], hdr['BPA'])
                    found = True
                except:
                    ### try see if AIPS as put the beam in HISTORY as usual
                   for h in hdr.get_history():
                      # Check if h is a string or a FITS Card object (long headers are
                      # split into Cards as of PyFITS 3.0.4)
                      if not isinstance(h, str):
                        hstr = h.value                        
                      else:
                        hstr = h
                      if N.all(['BMAJ' in hstr, 'BMIN' in hstr, 'BPA' in hstr, 'CLEAN' in hstr]):
                        try:
                            dum, dum, dum, bmaj, dum, bmin, dum, bpa = hstr.split()
                        except ValueError:
                            try:
                                dum, dum, bmaj, dum, bmin, dum, bpa, dum, dum = hstr.split()
                            except ValueError:
                                break    
                        beam = (float(bmaj), float(bmin), float(bpa))
                        found = True
            if not found: raise RuntimeError("No beam information found in image header.")

        ### convert beam into pixels and make sure it's asymmetric
        pbeam = beam2pix(beam)
        pbeam = (pbeam[0]/fwsig, pbeam[1]/fwsig, pbeam[2])  # IN SIGMA UNITS
        
        ### and store it
        img.pix2beam = pix2beam
        img.beam2pix = beam2pix
        img.pix2coord = pix2coord
        img.beam = beam   # FWHM size
        img.pixel_beam = pbeam   # IN SIGMA UNITS
        img.pixel_beamarea = 1.1331*img.pixel_beam[0]*img.pixel_beam[1]*fwsig*fwsig # area of restoring beam in pixels
        mylogger.userinfo(mylog, 'Beam shape (major, minor, pos angle)',
                          '(%s, %s, %s) degrees' % (round(beam[0],5),
                                                    round(beam[1],5),
                                                    round(beam[2],1)))

    def init_freq(self, img):
        """Initialize frequency parameters and store them"""
        mylog = mylogger.logging.getLogger("PyBDSM.InitFreq")
        if img.opts.frequency_sp != None and img.image.shape[1] > 1:
            # If user specifies multiple frequencies, then let
            # collapse.py do the initialization 
            img.cfreq = img.opts.frequency_sp[0]
            img.freq_pars = (0.0, 0.0, 0.0)
            mylog.info('Using user-specified frequency/frequencies.')
        elif img.opts.frequency != None:
            img.cfreq = img.opts.frequency
            img.freq_pars = (0.0, 0.0, 0.0)
            mylog.info('Using user-specified frequency/frequencies.')           
        else:
            found  = False
            hdr = img.header
            if img.use_io == 'rap':
                if hdr['coordinates'].has_key('spectral2'):
                    found = True
                    spec_dict = hdr['coordinates']['spectral2']['wcs']
                    crval, cdelt, crpix = spec_dict.get('crval'), \
                        spec_dict.get('cdelt'), spec_dict.get('crpix')
                    ff = crval+cdelt*(1.-crpix)

            if img.use_io == 'fits':
                nax = hdr['naxis']
                if nax > 2:
                    for i in range(nax):
                        s = str(i+1)
                        if hdr['ctype'+s][0:4] == 'FREQ':
                            found = True
                            crval, cdelt, crpix = hdr['CRVAL'+s], \
                                hdr['CDELT'+s], hdr['CRPIX'+s]
                            ff = crval+cdelt*(1.-crpix)
            if found: 
                img.cfreq = ff
                img.freq_pars = (crval, cdelt, crpix)
            else:
                raise RuntimeError('No frequency information found in image header.')

    def get_equinox(self, img):
        """Gets the equinox from the header.

        Returns float year with code, where code is:
        1 - EQUINOX, EPOCH or RADECSYS keyword not found in header
        0 - EQUINOX found as a numeric value
        1 - EPOCH keyword used for equinox (not recommended)
        2 - EQUINOX found as  'B1950'
        3 - EQUINOX found as  'J2000'
        4 - EQUINOX derived from value of RADECSYS keyword
            'ICRS', 'FK5' ==> 2000,  'FK4' ==> 1950
        """
        code = -1
        year = None
        if img.use_io == 'rap':
            hdr = img.header['coordinates']['direction0']
            code = -1
            year = None
            if hdr.has_key('system'):
                year = hdr['system']
                if isinstance(year, str):     # Check for 'J2000' or 'B1950' values
                    tst = year[:1]
                    if (tst == 'J') or (tst == 'B'):
                        year = float(year[1:])
                        if tst == 'J': code = 3
                        if tst == 'B': code = 2 
                else:
                    code = 0
        if img.use_io == 'fits':
            hdr = img.header
            if hdr.has_key('EQUINOX'):
                year = hdr['EQUINOX']
                if isinstance(year, str):     # Check for 'J2000' or 'B1950' values
                    tst = year[:1]
                    if (tst == 'J') or (tst == 'B'):
                        year = float(year[1:])
                        if tst == 'J': code = 3
                        if tst == 'B': code = 2 
                else:
                    code = 0
            else:
                if hdr.has_key('EPOCH'): # Check EPOCH if EQUINOX not found
                    year = hdr['EPOCH']
                    code = 1
                else:
                    if hdr.has_key('RADECSYS'):
                        sys = hdr['RADECSYS']
                        code = 4 
                        if sys[:3] == 'ICR': year = 2000.0
                        if sys[:3] == 'FK5': year = 2000.0
                        if sys[:3] == 'FK4': year = 1950.0
        return year, code
