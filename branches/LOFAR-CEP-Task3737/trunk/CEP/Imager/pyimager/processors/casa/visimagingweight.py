import numpy
import lofar.pyimager.algorithms.constants as constants

class VisImagingWeight :
    def __init__( self, **kwargs ):
        
        try :
            weighttype = kwargs["weighttype"]
        except KeyError:
            raise RuntimeError("No weighttype defined")
        weighttypes = ["uniform", "radial", "natural", "robust"]
        if weighttype not in weighttypes:
            raise RuntimeError("Unknown weighttype: " + weighttype + "\n" + "weighttype should be one of " + str(weighttypes))

        self.weighttype = weighttype
        if weighttype == "natural":
            self.imaging_weight = self.weightNatural
        elif weighttype == "radial":
            self.imaging_weight = self.weightRadial
        elif weighttype == "uniform":
            self.imaging_weight = self.weightDensityDependent
        elif weighttype == "robust":
            self.imaging_weight = self.weightDensityDependent
            try:
                rmode = kwargs["rmode"]
            except KeyError:
                RuntimeError("Robust weighting requires rmode parameter")
            if rmode not in ["abs", "normal"] :
                raise RuntimeError("Unknown rmode: " + mode + "\n" + "rmode should be one of " + str(rmodes))
            self.rmode = rmode
            if rmode == "abs":
                try:
                    self.noise = kwargs["noise"]
                except KeyError:
                   raise RuntimeError("Robust weighting with rmode = abs requires noise parameter")
            elif rmode == "normal":
                try:
                    self.robustness = kwargs["robustness"]
                except KeyError:
                    raise RuntimeError("Robust weighting with rmode = normal requires robustness parameter")
    
    def weightNatural(self, uvw, freqs, flag, weight_spectrum):
        imaging_weight = weight_spectrum.mean(axis=2) * numpy.float32((1 - flag.any(axis=2)))
        return imaging_weight
        #raise RuntimeError("weightNatural not implemented")
    
    def weightRadial(self, uvw, freqs, flag, weight_spectrum):
        raise RuntimeError("weightRadial not implemented")
    
    def weightDensityDependent(self, uvw, freqs, flag, weight_spectrum):
        imaging_weight = weight_spectrum.mean(axis=2) * numpy.float32((1 - flag.any(axis=2)))
        f = freqs/constants.speed_of_light
        
        uorig = int(self.density.shape[1]/2)
        vorig = int(self.density.shape[0]/2)
        
        uscale = self.density.shape[1]*self.density_increment[1]
        vscale = self.density.shape[0]*self.density_increment[0]
        for i in range(uvw.shape[0]):
            u1 = uvw[i,0]*uscale
            v1 = uvw[i,1]*vscale
            for j in range(len(f)):
                u = int(u1*f[j])
                v = int(v1*f[j])
                if abs(u)<uorig and abs(v)<vorig : 
                    imaging_weight[i, j] = imaging_weight[i, j] / (self.density[vorig+v,uorig+u]*self.f2 + self.d2)
        return imaging_weight
        
        
          
    def set_density( self, density, coordinates):
        self.density = density
        self.density_increment = coordinates.get_increment()[2]
        if self.weighttype == "uniform":
            self.f2 = 1.0
            self.d2 = 0.0
        else:
            sumwt = numpy.sum(density)
            sumlocwt = numpy.sum(density*density)
            if self.rmode == "abs":
                self.f2 = self.robust**2
                self.d2 = 2 * self.noise
            elif self.rmode == "normal":
                self.f2 = (5.0*pow(10.0,-self.robustness))**2 / (sumlocwt / sumwt)
                self.d2 = 1.0
