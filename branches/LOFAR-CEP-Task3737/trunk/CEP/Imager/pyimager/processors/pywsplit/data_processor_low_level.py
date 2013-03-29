import os.path as path
import numpy
import numpy.fft
import lofar.casaimwrap
import pyrap.tables
from ...algorithms import util
from ...algorithms import constants
import mod_threadpool as threadpool
#import datetime

class DataProcessorLowLevel:
    def __init__(self, measurement, options):
        self._measurement = measurement
        self._ms = pyrap.tables.table(measurement, readonly = False)
        self._ms = self._ms.query("ANTENNA1 != ANTENNA2 && OBSERVATION_ID ==" \
            " 0 && FIELD_ID == 0 && DATA_DESC_ID == 0")

#        assert(options["weight_algorithm"] == WeightAlgorithm.NATURAL)
        self._data_column = "CORRECTED_DATA"
        self._options = options

        # TODO: Make these proper options.
        self._options["uv_min"] = self._options.get("uv_min", 0)
        self._options["uv_max"] = self._options.get("uv_max", 100000)
        self._options["time_window"] = self._options.get("time_window", 300)
        self._options["oversample"] = self._options.get("oversample", 8)

        # Defaults from awimager.
        parms = {}
        parms["wmax"] = self._options["w_max"]
        parms["mueller.grid"] = numpy.ones((4, 4), dtype=bool)
        parms["mueller.degrid"] = numpy.ones((4, 4), dtype=bool)
        parms["verbose"] = 0                # 1, 2 for more output
        parms["maxsupport"] = 1024
        parms["oversample"] = self._options["oversample"]
        parms["imagename"] = self._options["image"]
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
        parms["padding"] = self._options["padding"]
        # will be determined by LofarFTMachine
        parms["wplanes"] = 0

#        parms["ApplyBeamCode"] = 0
        parms["ApplyBeamCode"] = 3
        parms["UVmin"] = self._options["uv_min"]
        parms["UVmax"] = self._options["uv_max"]
        parms["MakeDirtyCorr"] = False

        parms["timewindow"] = self._options["time_window"]
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

    def reference_frequency(self):
        spw = pyrap.tables.table(path.join(self._measurement, \
            "SPECTRAL_WINDOW"))
        return spw.getcell("REF_FREQUENCY", 0)

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
#        self._ms.putcol(self._data_column, self._degrid(coordinates, model))

    def residual(self, coordinates, model, density, as_grid):
        assert(density == 1.0)
        assert(not as_grid)

        # Degrid model.
        model_vis = self._degrid(coordinates, model)

        # Compute residual.
        residual = self._ms.getcol(self._data_column) - model_vis

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

    def _degrid(self, coordinates, model):
        antenna1 = self._ms.getcol("ANTENNA1")
        antenna2 = self._ms.getcol("ANTENNA2")
        uvw = self._ms.getcol("UVW")
        time_centroid = self._ms.getcol("TIME_CENTROID")
        flag = self._ms.getcol("FLAG")

        ref_freq = self.reference_frequency()
        ch_freq = self.channel_frequency()

        # Initialize LOFAR::LofarConvolutionFunction machinery.
        lofar.casaimwrap.init_cf(self._context, model.shape, coordinates.dict())
        lofar.casaimwrap.init_aterm(self._context, time_centroid)

        spheroid = lofar.casaimwrap.spheroid(self._context)
        spheroid = numpy.where(spheroid >= 1e-2, 1.0 / numpy.square(spheroid),
            0.0)

        # Convert model image to linear correlations and divide by the square
        # of the spheroid function to account for the spheroid functions
        # applied as part of the AW-projection.
        #
        # TODO: Check if the spheroid is also included in the squared response
        # image, and if so how to handle the normalization correctly.
        model = lofar.casaimwrap.stokes_to_linear(self._context,
            coordinates.dict(), model * spheroid)

        # Create an index of spans of visibility data that can be gridded
        # independently.
        w_map, w_index_map = self._make_mapping_time_W(antenna1, antenna2, uvw,
            time_centroid, ref_freq, self._options["time_window"],
            self._options["uv_min"], self._options["uv_max"],
            self._options["w_max"])

        # Comment from CASA source code:
        #
        # NEGATING to correct for an image inversion problem.
        #
        uvw[:, :2] *= -1.0

        # Get coordinate increments.
        #
        # TODO: Remove assumptions about which coordinate axis represents what.
        #
        inc_ra = coordinates.get_increment()[-1][-1]
        inc_dec = coordinates.get_increment()[-1][0]

        # LOFAR::LofarFTMachine forces oversampling factor to the next higher
        # odd number if it is even.
        oversample = self._options["oversample"]
        if oversample % 2 == 0:
            oversample += 1

        # Allocate buffer for the computed visibility data.
        vis = numpy.zeros(flag.shape, dtype = numpy.complex64)

        # Degrid.
        # Process each W-plane sequentially, processing all the spans of a
        # single W-plane in parallel.
        #
        pool = threadpool.ThreadPool(self._options.get("threads", 1))
        for i in range(len(w_map)):
            active_w_map = w_map[i]
            if len(active_w_map) == 0:
                continue

            # NB. Applying the W-term and FFT does not seem to take much time.
            #
            wcorr = lofar.casaimwrap.apply_w_term_image(self._context, model,
                w_index_map[i])

            # TODO: Without fftshift the result does not match the reference
            # implementation (LofarFTMachine::getSplitWplanes). This is true for
            # images of even sizes, not sure for odd sizes.
            #
            wcorr = numpy.fft.fft2(numpy.fft.fftshift(wcorr, (2, 3)))

            # NB. Cast back to complex64 to avoid unwanted copies in the pyrap
            # python-to-C++ conversion layer.
            #
            wcorr = wcorr.astype(numpy.complex64)

            def _process_span(thread, index):
                """Helper function to processes spans in parallel."""

                assert(len(active_w_map[index]) > 0)
                start = active_w_map[index][0]
                end = active_w_map[index][-1]
                time_mean = 0.5 * (time_centroid[start] + time_centroid[end])
                w_mean = 0.5 * (uvw[start, 2] + uvw[end, 2])

                # Create convolution kernel.
                kernel = \
                    lofar.casaimwrap.make_convolution_function(self._context,
                        thread, antenna1[start], antenna2[start], time_mean,
                        w_mean)

                # Degrid.
                lofar.casaimwrap.degrid_reimplemented(inc_ra, inc_dec,
                    oversample, wcorr, kernel, uvw, ch_freq, flag, vis,
                    active_w_map[index])

            # Multi-threaded execution of _process_plane() for all spans in this
            # W-plane.
            map(pool.putRequest, threadpool.makeRequests(_process_span,
                range(len(active_w_map))))

#            start = datetime.datetime.now()
            pool.wait()
#            end = datetime.datetime.now()
#            delta = end - start
#            print "threads:", threads, "use_processes:", use_processes,
#                "time:", delta.seconds + delta.microseconds / 1e6
        return vis

    def _make_mapping_time(self, antenna1, antenna2, uvw, time, ref_freq,
        time_window, uv_min, uv_max, w_max):
        """Re-implementation of LofarFTMachine::make_mapping_time()."""

        mapping = []
        sub_mapping = []

        ref_wl = constants.speed_of_light / ref_freq

        # The last array passed to numpy.lexsort() is the primary sort key,
        # hence the reversal of antenna1 and antenna2.
        index = numpy.lexsort((antenna2, antenna1))
        active_baseline = (antenna1[index[0]], antenna2[index[0]])
        active_window = time[index[0]] + time_window

        for row in index:
            if numpy.abs(uvw[row, 2]) >= w_max:
                continue

            # Compute UV distance in klambda.
            uv_distance = numpy.sqrt(numpy.sum(numpy.square(uvw[row,:2]))) / \
                (1e3 * ref_wl)
            if uv_distance <= uv_min or uv_distance >= uv_max:
                continue

            baseline = (antenna1[row], antenna2[row])
            if time[row] > active_window or baseline != active_baseline:
                active_window = time[row] + time_window
                active_baseline = baseline
                mapping.append(sub_mapping)
                sub_mapping = []

            sub_mapping.append(row)
        mapping.append(sub_mapping)
        return mapping

    def _make_mapping_time_W(self, antenna1, antenna2, uvw, time, ref_freq,
        time_window, uv_min, uv_max, w_max):
        """Re-implementation of LofarFTMachine::make_mapping_time()_W."""

        tmp = lofar.casaimwrap.w_index(self._context, uvw[:,2], 0)
        w_index = tmp["w_index"]

        ref_wl = constants.speed_of_light / ref_freq

        # The last array passed to numpy.lexsort() is the primary sort key,
        # hence the reversal of antenna1 and antenna2.
        index = numpy.lexsort((antenna2, antenna1, w_index))
        active_baseline = None
        active_window = None
        active_w_index = None

        w_map = []
        w_index_map = []
        sub_map = []
        sub_w_map = []

        for row in index:
            if numpy.abs(uvw[row, 2]) >= w_max:
                continue

            # Compute UV distance in klambda.
            uv_distance = numpy.sqrt(numpy.sum(numpy.square(uvw[row,:2]))) / \
                (1e3 * ref_wl)
            if uv_distance <= uv_min or uv_distance >= uv_max:
                continue

            if active_baseline is None:
                active_baseline = (antenna1[row], antenna2[row])
                active_window = time[row] + time_window
                active_w_index = w_index[row]

            baseline = (antenna1[row], antenna2[row])
            if time[row] > active_window or baseline != active_baseline:
                active_window = time[row] + time_window
                active_baseline = baseline
                sub_w_map.append(sub_map)
                sub_map = []

            if w_index[row] != active_w_index:
                w_map.append(sub_w_map)
                w_index_map.append(active_w_index)

                active_w_index = w_index[row]
                sub_w_map = []

            sub_map.append(row)

        sub_w_map.append(sub_map)
        w_map.append(sub_w_map)
        w_index_map.append(active_w_index)

        return (w_map, w_index_map)
