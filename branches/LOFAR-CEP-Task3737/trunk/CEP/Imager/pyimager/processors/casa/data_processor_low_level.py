from ..data_processor_low_level_base import DataProcessorLowLevelBase
import itertools
import os.path as path
import numpy
import lofar.casaimwrap
import lofar.pyimager.algorithms.constants as constants
import pyrap.tables
import visimagingweight

class DataProcessorLowLevel(DataProcessorLowLevelBase):
    def __init__(self, measurement, options):
        print measurement
        self._measurement = measurement
        self._ms = pyrap.tables.table(measurement)
        self._ms = self._ms.query("ANTENNA1 != ANTENNA2 && OBSERVATION_ID ==" \
            " 0 && FIELD_ID == 0 && DATA_DESC_ID == 0")

#        assert(options["weight_algorithm"] == WeightAlgorithm.NATURAL)
        self._data_column = "CORRECTED_DATA"

        self._coordinates = None
        self._shape = None
        self._response_available = False
        
        # Defaults from awimager.
        parms = {}
        parms["wmax"] = options["w_max"]
        parms["mueller.grid"] = numpy.ones((4, 4), dtype=bool)
        parms["mueller.degrid"] = numpy.ones((4, 4), dtype=bool)
        parms["verbose"] = 0                # 1, 2 for more output
        parms["maxsupport"] = 1024
        parms["oversample"] = 8
        parms["imagename"] = options["image"]
        parms["UseLIG"] = False             # linear interpolation
        parms["UseEJones"] = True
        parms["ApplyElement"] = True
        parms["PBCut"] = 5e-2
        parms["StepApplyElement"] = 1000       # if 0 don't apply element beam
#        parms["TWElement"] = 0.02
        parms["PredictFT"] = False
        parms["PsfImage"] = ""
        parms["UseMasksDegrid"] = True
        parms["RowBlock"] = 10000000
        parms["doPSF"] = False
        parms["applyIonosphere"] = False
        parms["applyBeam"] = True
        parms["splitbeam"] = True
        parms["padding"] = options["padding"]
        # will be determined by LofarFTMachine
        parms["wplanes"] = 0

        parms["ApplyBeamCode"] = 0
        #parms["ApplyBeamCode"] = 3
        parms["UVmin"] = 0
        parms["UVmax"] = 100000
        parms["MakeDirtyCorr"] = False

        parms["timewindow"] = 300
        parms["TWElement"] = 20
        parms["UseWSplit"] = True
        parms["SingleGridMode"] = True
        parms["SpheSupport"] = 15
        parms["t0"] = -1
        parms["t1"] = -1
        parms["ChanBlockSize"] = 0
        parms["FindNWplanes"] = True
        
        weightoptionnames = ["weighttype", "rmode", "noise", "robustness"]
        weightoptions = dict( (key, value) for (key,value) in options.iteritems() if key in weightoptionnames)
        self.imw = visimagingweight.VisImagingWeight(**weightoptions)

        self._context = lofar.casaimwrap.CASAContext()
        lofar.casaimwrap.init(self._context, self._measurement, parms)

    def capabilities(self):
        return {}

    def phase_reference(self):
        field = pyrap.tables.table(path.join(self._measurement, "FIELD"))
        # Assumed to be in J2000 for now.
        assert(field.getcolkeyword("PHASE_DIR", "MEASINFO")["Ref"] == "J2000")
        return field.getcell("PHASE_DIR", 0)[0]

    def channel_frequency(self):
        spw = pyrap.tables.table(path.join(self._measurement, \
            "SPECTRAL_WINDOW"))
        return spw.getcell("CHAN_FREQ", 0)

    def channel_width(self):
        spw = pyrap.tables.table(path.join(self._measurement, \
            "SPECTRAL_WINDOW"))
        return spw.getcell("CHAN_WIDTH", 0)

    def maximum_baseline_length(self):
        return numpy.max(numpy.sqrt(numpy.sum(numpy.square( \
            self._ms.getcol("UVW")), 1)))

    def density(self, coordinates, shape):
        increment = coordinates.get_increment()
        freqs = self.channel_frequency()
        f = freqs/constants.speed_of_light
        
        density_shape = shape[2:]
        density_increment = increment[2]
        
        uorig = int(density_shape[1]/2)
        vorig = int(density_shape[0]/2)
        
        density = numpy.zeros(density_shape)
        uscale = density_shape[1]*density_increment[1]
        vscale = density_shape[0]*density_increment[0]
        uvw = self._ms.getcol("UVW")
        weight = self._ms.getcol("WEIGHT_SPECTRUM")
        for i in range(len(self._ms)):
          u1 = uvw[i,0]*uscale
          v1 = uvw[i,1]*vscale
          for j in range(len(f)):
              u = int(u1*f[j])
              v = int(v1*f[j])
              if abs(u)<uorig and abs(v)<vorig : 
                  w = sum(weight[i, j,:])
                  density[vorig+v,uorig+u] += w
                  density[vorig-v,uorig-u] += w
        return density
        
    def set_density(self, density, coordinates) :
        self.imw.set_density(density, coordinates)

    def response(self, coordinates, shape):
        self._update_image_configuration(coordinates, shape)
        assert(self._response_available)
        return lofar.casaimwrap.average_response(self._context)

    def point_spread_function(self, coordinates, shape, as_grid):
        assert(not as_grid)
        self._update_image_configuration(coordinates, shape)

        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["FLAG"] = self._ms.getcol("FLAG")
        args["IMAGING_WEIGHT"] = self.imw.imaging_weight(args["UVW"], \
            self.channel_frequency(), args["FLAG"], self._ms.getcol("WEIGHT_SPECTRUM"))
        args["DATA"] = numpy.ones(args["FLAG"].shape, dtype=numpy.complex64)

        lofar.casaimwrap.begin_grid(self._context, shape, coordinates.dict(), \
            True, args)
        result = lofar.casaimwrap.end_grid(self._context, False)
        return (result["image"], result["weight"])

    def grid(self, coordinates, shape, as_grid):
        assert(not as_grid)

        self._update_image_configuration(coordinates, shape)
        print  "****************************************************************"
        print coordinates
        print shape
        print  "****************************************************************"

        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["FLAG"] = self._ms.getcol("FLAG")
        args["IMAGING_WEIGHT"] = numpy.ones(args["FLAG"].shape[:2], dtype=numpy.float32)
        args["DATA"] = self._ms.getcol(self._data_column)

        lofar.casaimwrap.begin_grid(self._context, shape, coordinates.dict(), \
            False, args)
        result = lofar.casaimwrap.end_grid(self._context, False)
        self._response_available = True
        return (result["image"], result["weight"])

    def degrid(self, coordinates, model, as_grid):
        assert(not as_grid)
        self._update_image_configuration(coordinates, model.shape)

        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["FLAG"] = self._ms.getcol("FLAG")
        args["IMAGING_WEIGHT"] = numpy.ones(args["FLAG"].shape[:2], dtype=numpy.float32)

        result = lofar.casaimwrap.begin_degrid(self._context, \
            coordinates.dict(), model, args)
        lofar.casaimwrap.end_degrid(self._context)
        self._response_available = True
#        self._ms.putcol(self._data_column, result["data"])

    def residual(self, coordinates, model, as_grid):
        assert(not as_grid)
        self._update_image_configuration(coordinates, model.shape)

        # Degrid model.
        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["FLAG"] = self._ms.getcol("FLAG")
        args["IMAGING_WEIGHT"] = numpy.ones(args["FLAG"].shape[:2], dtype=numpy.float32)

        result = lofar.casaimwrap.begin_degrid(self._context, \
            coordinates.dict(), model, args)
        lofar.casaimwrap.end_degrid(self._context)

        # Compute residual.
        residual = self._ms.getcol(self._data_column) - result["data"]

        # Grid residual.
        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["FLAG"] = self._ms.getcol("FLAG")
        args["IMAGING_WEIGHT"] = numpy.ones(args["FLAG"].shape[:2], dtype=numpy.float32)
        args["DATA"] = residual

        lofar.casaimwrap.begin_grid(self._context, model.shape, \
            coordinates.dict(), False, args)
        result = lofar.casaimwrap.end_grid(self._context, False)
        self._response_available = True

        return (result["image"], result["weight"])
        
    def _update_image_configuration(self, coordinates, shape):
        if self._coordinates != coordinates or self._shape != shape:
            self._coordinates = coordinates
            self._shape = shape
            self._response_available = False
