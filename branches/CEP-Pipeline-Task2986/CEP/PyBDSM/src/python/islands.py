"""Module islands.

Defines operation Op_islands which does island detection.
Current implementation uses scipy.ndimage operations for island detection.
While it's implemented to work for images of arbitrary dimensionality, 
the bug in the current version of scipy (0.6) often causes crashes 
(or just wrong results) for 3D inputs.

If this (scipy.ndimage.label) isn't fixed by the time we need 3D source 
extraction, one will have to adopt my old pixel-runs algorithm for 3D data. 
Check out islands.py rev. 1362 from repository for it.
"""

import numpy as N
import scipy.ndimage as nd
from image import *
import mylogger
import pyfits
import functions as func
from output import write_islands
from readimage import Op_readimage
from preprocess import Op_preprocess
from rmsimage import Op_rmsimage
from threshold import Op_threshold
from collapse import Op_collapse

nisl = Int(doc="Total number of islands detected")

class Op_islands(Op):
    """Detect islands of emission in the image

    All detected islands are stored in the list img.islands,
    where each individual island is represented as an instance 
    of class Island.
    
    The option to detect islands on a different "detection" 
    image is also available. This option is useful for example
    when a primary beam correction is used -- it is generally
    better to detect sources on the uncorrected image, but 
    to measure them on the corrected image.

    Prerequisites: module rmsimage should be run first.
    """
    def __call__(self, img):
        mylog = mylogger.logging.getLogger("PyBDSM."+img.log+"Islands")
        opts = img.opts

        if opts.detection_image != '':
            # Use a different image for island detection. The detection
            # image and the measurement image must have the same shape
            # and be registered. Otherwise, one could reproject the
            # detection image using, e.g., the Kapteyn package.
            #
            # First, set up up an Image object and run a limited
            # op_chain.
            from . import _run_op_list
            mylogger.userinfo(mylog, "\nDetermining islands from detection image")

            det_chain, det_opts = self.setpara_bdsm(img, opts.detection_image)
            det_img = Image(det_opts)
            det_img.log = 'Detection image'            
            success = _run_op_list(det_img, det_chain)                    
            if not success:
                return

            # Check that the ch0 images are the same size
            det_shape = det_img.ch0.shape
            ch0_shape = img.ch0.shape
            if det_shape != ch0_shape:
                raise RuntimeError("Detection image shape does not match that of input image.")
            
            # Run through islands and correct the rms, mean and max values
            img.island_labels = det_img.island_labels
            corr_islands = []
            for isl in det_img.islands:
                corr_islands.append(isl.copy(img))
            img.islands = corr_islands
            img.nisl = len(img.islands)
            img.pyrank = det_img.pyrank
            
        else:
            img.islands = self.ndimage_alg(img, opts)
            img.nisl = len(img.islands)
    
            mylogger.userinfo(mylog, "Number of islands found", '%i' %
                              len(img.islands))
            
            pyrank = N.zeros(img.ch0.shape, dtype=int) - 1
            for i, isl in enumerate(img.islands):
                isl.island_id = i
                if i == 0:
                    pyrank[isl.bbox] = N.invert(isl.mask_active)-1
                else:
                    pyrank[isl.bbox] = N.invert(isl.mask_active)*i
                
            if opts.output_all: write_islands(img)
            if opts.savefits_rankim:
                func.write_image_to_file(img.use_io, img.imagename + '_pyrank.fits', pyrank, img)

            img.pyrank = pyrank
            
        img.completed_Ops.append('islands')
        return img

    def ndimage_alg(self, img, opts):
        """Island detection using scipy.ndimage

        Use scipy.ndimage.label to detect islands of emission in the image.
        Island is defined as group of tightly connected (8-connectivity 
        for 2D images) pixels with emission.

        The following cuts are applied:
         - pixel is considered to have emission if it is 'thresh_isl' times 
           higher than RMS.
         - Island should have at least 'minsize' active pixels
         - There should be at lease 1 pixel in the island which is 'thresh_pix'
           times higher than noise (peak clip).

        Parameters:
        image, mask: arrays with image data and mask
        mean, rms: arrays with mean & rms maps
        thresh_isl: threshold for 'active pixels'
        thresh_pix: threshold for peak
        minsize: minimal acceptable island size

        Function returns a list of Island objects.
        """
        ### islands detection
        mylog = mylogger.logging.getLogger("PyBDSM."+img.log+"Islands")

        image = img.ch0 
        mask = img.mask
        rms = img.rms
        mean = img.mean
        thresh_isl = opts.thresh_isl
        thresh_pix = img.thresh_pix
        minsize = opts.minpix_isl
        if minsize == None:
            minsize = int(img.pixel_beamarea/3.0) # 1/3 of beam area in pixels
            if minsize < 6:
                minsize = 6 # Need at least 6 pixels to obtain good fits
            mylogger.userinfo(mylog, "Minimum number of pixels per island", '%i' %
                          minsize)
        
        clipped_mean = img.clipped_mean
        saverank = opts.savefits_rankim

                        # act_pixels is true if significant emission
        act_pixels = (image-mean)/thresh_isl >= rms
        if isinstance(mask, N.ndarray):
            act_pixels[mask] = False

                        # dimension of image
        rank = len(image.shape)
                        # generates matrix for connectivity, in this case, 8-conn
        connectivity = nd.generate_binary_structure(rank, rank)
                        # labels = matrix with value = (initial) island number
        labels, count = nd.label(act_pixels, connectivity)
                        # slices has limits of bounding box of each such island
        slices = nd.find_objects(labels)
        img.island_labels = labels

        ### apply cuts on island size and peak value
        pyrank = N.zeros(image.shape)
        res = []
        islid = 0
        for idx, s in enumerate(slices):
            idx += 1 # nd.labels indices are counted from 1
                        # number of pixels inside bounding box which are in island
            isl_size = (labels[s] == idx).sum()
            isl_peak = nd.maximum(image[s], labels[s], idx)
            isl_maxposn = tuple(N.array(N.unravel_index(N.argmax(image[s]), image[s].shape))+\
                          N.array((s[0].start, s[1].start)))
            if (isl_size >= minsize) and (isl_peak - mean[isl_maxposn])/thresh_pix > rms[isl_maxposn]:
                isl = Island(image, mask, mean, rms, labels, s, idx)
                res.append(isl)
                pyrank[isl.bbox] += N.invert(isl.mask_active)*idx / idx

        return res

    def setpara_bdsm(self, img, det_file):
        from types import ClassType, TypeType

        chain=[Op_readimage(), Op_collapse(), Op_preprocess, Op_rmsimage(), Op_threshold(), Op_islands()]
        opts = img.opts.to_dict()
        opts['filename'] = det_file
        opts['detection_image'] = ''
        opts['polarisation_do'] = False
        
        ops = []
        for op in chain:
          if isinstance(op, (ClassType, TypeType)):
            ops.append(op())
          else:
            ops.append(op)

        return ops, opts


from image import *

class Island(object):
    """Instances of this class represent islands of emission in the image.

    Its primary use is a container for all kinds of data describing island.
    """
    bbox        = List(Instance(slice(0), or_none=False),
                       doc = "Bounding box of the island")
    origin      = List(Float(), doc="Coordinates of lower-left corner")
    image       = NArray(doc="Sub-image of the island")
    mask_active = NArray(doc="Mask for just active pixels")
    mask_noisy  = NArray(doc="Mask for active pixels and surrounding noise")
    shape       = List(Int(), doc="Shape of the island")
    size_active = Int(doc="Number of active pixels in the island")
    mean        = Float(doc="Average mean value")
    rms         = Float(doc="Average rms")
    max_value   = Float(doc="Maximum value in island")
    island_id   = Int(doc="Island id, starting from 0", colname='Isl_id')
    gresid_rms  = Float(doc="Rms of residual image of island")
    gresid_mean = Float(doc="Mean of residual image of island")
    connected   = Tuple(String(), Int(), doc="'multiple' or 'single' -ly connected, # of holes inside island")
    convex_def  = Float(doc="Convex deficiency, with first order correction for edge effect")
    islmean     = Float(doc="a constant value to subtract from image before fitting")

    def __init__(self, img, mask, mean, rms, labels, bbox, idx):
        """Create Island instance.

        Parameters:
        img, mask, mean, rms: arrays describing image
        labels: labels array from scipy.ndimage
        bbox: slices
        """
        TCInit(self)
        
        ### we make bbox slightly bigger
        self.oldbbox = bbox
        self.oldidx = idx
        bbox = self.__expand_bbox(bbox, img.shape)
        origin = [b.start for b in bbox]   # easier in case ndim > 2
        data = img[bbox]

        ### create (inverted) masks
        # Note that mask_active is the island mask; mask_noisy is just
        # the noisy pixels in the island image. If you want to mask the
        # noisy pixels as well, set the mask to:
        #     mask = mask_active + mask_noisy
        isl_mask = (labels[bbox] == idx)
        noise_mask = (labels[bbox] == 0)
        N.logical_or(noise_mask, isl_mask, noise_mask)

        isl_size = isl_mask.sum()

        ### invert masks
        N.logical_not(isl_mask, isl_mask)
        N.logical_not(noise_mask, noise_mask)
        if isinstance(mask, N.ndarray):
            noise_mask[mask[bbox]] = True

        ### finish initialization
        self.bbox = bbox
        self.origin = origin
        self.image = data
        self.mask_active = isl_mask
        self.mask_noisy = noise_mask
        self.shape = data.shape
        self.size_active = isl_size
        self.max_value = N.max(self.image*~self.mask_active)
        bbox_rms_im = rms[bbox]
        in_bbox_and_unmasked = N.where(~N.isnan(bbox_rms_im))
        self.rms  = bbox_rms_im[in_bbox_and_unmasked].mean()
        bbox_mean_im = mean[bbox]
        in_bbox_and_unmasked = N.where(~N.isnan(bbox_mean_im))
        self.mean  = bbox_mean_im[in_bbox_and_unmasked].mean()
        self.border = self.get_border()

    ### do map etc in case of ndim image
    def __expand_bbox(self, bbox, shape):
        """Expand bbox of the image by 1 pixel"""
        def __expand(bbox, shape):
            return slice(max(0, bbox.start - 1), min(shape, bbox.stop + 1))
        return map(__expand, bbox, shape) 

    def copy(self, img):
        mask, mean, rms = img.mask, img.mean, img.rms
        image = img.ch0; labels = img.island_labels; bbox = self.oldbbox; idx = self.oldidx
        return Island(image, mask, mean, rms, labels, bbox, idx)

    def get_border(self):
        """ From all valid island pixels, generate the border."""
        mask = ~self.mask_active
        border = N.transpose(N.asarray(N.where(mask - nd.binary_erosion(mask)))) + self.origin
        
        return N.transpose(N.array(border))


### Insert attribute for island list into Image class
Image.islands = List(tInstance(Island), doc="List of islands")


