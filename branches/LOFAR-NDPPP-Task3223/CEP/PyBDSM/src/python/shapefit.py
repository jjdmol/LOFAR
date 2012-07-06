"""Module shapelets

This will do all the shapelet analysis of islands in an image
"""

from image import *
from islands import *
from shapelets import *
import mylogger
import statusbar


Island.shapelet_basis=String(doc="Coordinate system for shapelet decomposition (cartesian/polar)", colname='Basis', units=None)
Island.shapelet_beta=Float(doc="Value of shapelet scale beta", colname='Beta', units=None)
Island.shapelet_nmax=Int(doc="Maximum value of shapelet order", colname='NMax', units=None)
Island.shapelet_centre=Tuple(Float(), Float(),doc="Centre for the shapelet decomposition, starts from zero")
Island.shapelet_cf=NArray(doc="Coefficient matrix of the shapelet decomposition", colname='Coeff_matrix', units=None)

class Op_shapelets(Op):
    """ Get the image and mask from each island and send it to 
    shapelet programs which can then also be called seperately """

    def __call__(self, img):
    
        mylog = mylogger.logging.getLogger("PyBDSM."+img.log+"Shapefit")
        global bar
        bar = statusbar.StatusBar('Decomposing islands into shapelets ...... : ', 0, img.nisl)
        if img.opts.shapelet_do:
            if img.opts.quiet == False:
                bar.start()
            for id, isl in enumerate(img.islands):
                arr=isl.image
                mask=isl.mask_active + isl.mask_noisy
                basis=img.opts.shapelet_basis
                beam_pix=img.beam2pix(img.beam)
                mode=img.opts.shapelet_fitmode
                if mode != 'fit': mode=''

                fixed=(0,0,0)
                (beta, centre, nmax)=self.get_shapelet_params(arr, mask, basis, beam_pix, fixed, N.array(isl.origin), mode)

                cf=decompose_shapelets(arr, mask, basis, beta, centre, nmax, mode)

                isl.shapelet_beta=beta
                isl.shapelet_centre=tuple(N.array(centre) + N.array(isl.origin))
                isl.shapelet_nmax=nmax
                isl.shapelet_basis=basis
                isl.shapelet_cf=cf
                mylog.info('Shape : cen '+str(isl.shapelet_centre[0])+' '+ \
                     str(isl.shapelet_centre[1])+' beta '+str(beta))
                if img.opts.quiet == False:
                    bar.increment()
            img.completed_Ops.append('shapelets')


    def get_shapelet_params(self, image, mask, basis, beam_pix, fixed, ori, mode, beta=None, cen=None, nmax=None):
         """ This takes as input an image, its mask (false=valid), basis="cartesian"/"polar", 
	     fixed=(i,j,k) where i,j,k =0/1 to calculate or take as fixed for (beta, centre, nmax),
	     beam_pix has the beam in (pix_fwhm, pix_fwhm, deg), 
	     beta (the scale), cen (centre of basis expansion), nmax (max order). The output
	     is an updated set of values of (beta, centre, nmax). If fixed is 1 and the value is not
	     specified as an argument, then fixed is taken as 0."""
	 from math import sqrt, log, floor
         import functions as func
         import numpy as N
         
	 if fixed[0]==1 and beta==None: fixed[0]=0
	 if fixed[1]==1 and cen==None: fixed[1]=0
	 if fixed[2]==1 and nmax==None: fixed[2]=0

         if fixed[0]*fixed[1]==0:
             (m1, m2, m3)=func.moment(image, mask)
             
         if fixed[0]==0:
             beta=sqrt(m3[0]*m3[1])*2.0
             if beta == 0.0:
                beta = 0.5
	 if fixed[1]==0:
             cen=m2
         if fixed[2]==0:
             (n, m)=image.shape
             nmax=int(round(sqrt(1.0*n*n+m*m)/beam_pix[1]))-1
             nmax=min(max(nmax*2+2,10),10)                      # totally ad hoc
             npix = N.product(image.shape)-N.sum(mask)
             if nmax*nmax >= n*m : nmax = int(floor(sqrt(npix-1)))  # -1 is for when n*m is a perfect square
             if mode == 'fit':   # make sure npara <= npix
               nmax_max = int(round(0.5*(-3+sqrt(1+8*npix))))
               nmax=min(nmax, nmax_max)

         betarange=[0.5,sqrt(beta*max(n,m))]  # min, max
         #print betarange 

         #print 'Initial Beta = ',beta, image.shape

	 if fixed[1]==0:
             cen=shape_findcen(image, mask, basis, beta, nmax, beam_pix) # + check_cen_shapelet
         #print 'First Centre = ',cen,N.array(cen)+ori

         from time import time
         t1 = time()
         if fixed[0]==0:
             beta, err=shape_varybeta(image, mask, basis, beta, cen, nmax, betarange, plot=False)
         t2 = time()
         #print 'TIME ',t2-t1, '\n'
         #print 'Final Beta = ',beta, err

	 if fixed[1]==0 and fixed[0]==0:
             cen=shape_findcen(image, mask, basis, beta, nmax, beam_pix) # + check_cen_shapelet
         #print 'Final Cen = ',N.array(cen)+ori

         return beta, cen, nmax

 
       

