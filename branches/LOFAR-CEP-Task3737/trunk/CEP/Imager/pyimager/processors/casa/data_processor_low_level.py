import os.path as path
import numpy
import lofar.casaimwrap
import pyrap.tables

class DataProcessorLowLevel:
    def __init__(self, measurement, options):
        self._measurement = measurement
        self._ms = pyrap.tables.table(measurement)
        self._ms = self._ms.query("ANTENNA1 != ANTENNA2 && OBSERVATION_ID ==" \
            " 0 && FIELD_ID == 0 && DATA_DESC_ID == 0")

#        assert(options["weight_algorithm"] == WeightAlgorithm.NATURAL)
        self._data_column = "CORRECTED_DATA"

        # Defaults from awimager.
        parms = {}
        parms["wmax"] = options["wmax"]
        parms["mueller.grid"] = numpy.ones((4, 4), dtype=bool)
        parms["mueller.degrid"] = numpy.ones((4, 4), dtype=bool)
        parms["verbose"] = 0                # 1, 2 for more output
        parms["maxsupport"] = 1024
        parms["oversample"] = 8
        parms["imagename"] = options["image"]
        parms["UseLIG"] = False             # linear interpolation
        parms["UseEJones"] = True
        #parms["ApplyElement"] = True
        parms["PBCut"] = 1e-2
        parms["StepApplyElement"] = 0       # if 0 don't apply element beam
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

#        parms["ApplyBeamCode"] = 0
        parms["ApplyBeamCode"] = 3
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
        return 1.0

    def response(self, coordinates, shape, density):
        # TODO: This is a hack! LofarFTMachine computes the average response
        # while gridding. It cannot compute it on its own for arbitrary
        # coordinates and shape. For the moment, the CASA DataProcessor class
        # ensures this function is always called with the same coordinates and
        # shape as were used in the last call to get().
        return lofar.casaimwrap.average_response(self._context)

    def point_spread_function(self, coordinates, shape, density, as_grid):
        assert(density == 1.0)
        assert(not as_grid)

        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["WEIGHT"] = self._ms.getcol("WEIGHT")
        args["FLAG"] = self._ms.getcol("FLAG")
        args["DATA"] = numpy.ones(args["FLAG"].shape, dtype=numpy.complex64)

        lofar.casaimwrap.begin_grid(self._context, shape, coordinates.dict(), \
            True, args)
        result = lofar.casaimwrap.end_grid(self._context, False)
        return (result["image"], result["weight"])

    def grid(self, coordinates, shape, density, as_grid):
        assert(density == 1.0)
        assert(not as_grid)

        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["WEIGHT"] = self._ms.getcol("WEIGHT")
        args["FLAG"] = self._ms.getcol("FLAG")
        args["DATA"] = self._ms.getcol(self._data_column)

        lofar.casaimwrap.begin_grid(self._context, shape, coordinates.dict(), \
            False, args)
        result = lofar.casaimwrap.end_grid(self._context, False)
        return (result["image"], result["weight"])

    def degrid(self, coordinates, model, as_grid):
        assert(not as_grid)

        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["WEIGHT"] = self._ms.getcol("WEIGHT")
        args["FLAG"] = self._ms.getcol("FLAG")

        result = lofar.casaimwrap.begin_degrid(self._context, \
            coordinates.dict(), model, args)
        lofar.casaimwrap.end_degrid(self._context)
#        self._ms.putcol(self._data_column, result["data"])

    def residual(self, coordinates, model, density, as_grid):
        assert(density == 1.0)
        assert(not as_grid)

        # Degrid model.
        args = {}
        args["ANTENNA1"] = self._ms.getcol("ANTENNA1")
        args["ANTENNA2"] = self._ms.getcol("ANTENNA2")
        args["UVW"] = self._ms.getcol("UVW")
        args["TIME"] = self._ms.getcol("TIME")
        args["TIME_CENTROID"] = self._ms.getcol("TIME_CENTROID")
        args["FLAG_ROW"] = self._ms.getcol("FLAG_ROW")
        args["WEIGHT"] = self._ms.getcol("WEIGHT")
        args["FLAG"] = self._ms.getcol("FLAG")

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
        args["WEIGHT"] = self._ms.getcol("WEIGHT")
        args["FLAG"] = self._ms.getcol("FLAG")
        args["DATA"] = residual

        lofar.casaimwrap.begin_grid(self._context, model.shape, \
            coordinates.dict(), False, args)
        result = lofar.casaimwrap.end_grid(self._context, False)

        return (result["image"], result["weight"])
