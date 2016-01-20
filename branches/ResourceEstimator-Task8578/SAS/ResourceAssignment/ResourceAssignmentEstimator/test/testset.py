""" test parameterset for resource estimator
"""

from lofar.sas.resourceassignment.resourceassignmentestimator.resource_estimators import ParameterSet
#from parameterset import ParameterSet

class TestSet(object):
    def __init__(self):
        self.check_set = ParameterSet()
        self.valid_set = ParameterSet()

    def clear(self):
        self.check_set.clear()
        self.valid_set.clear()

    def test_dict(self):
        return self.check_set.get_set()

    def valid_dict(self):
        return self.valid_set.get_set()

    # for observation
    def add_observation(self):
        checkset = """
        observation.sample_clock= 200
        observation.duration=3600
        observation.channels_per_subband= 61
        observation.intergration_time= 1
        observation.antenna_mode= HBA_DUAL
        observation.stations= [CS001, CS002, RS307, RS509]
        observation.flys_eye.enabled= false
        observation.nr_beams= 0

        # coherent_stokes.type can be: DATA_TYPE_XXYY or DATA_TYPE_STOKES_IQUV
        observation.output.coherent_stokes.enabled= false
        observation.output.coherent_stokes.type=
        observation.output.coherent_stokes.integration_factor=

        # incoherent_stokes.type can be: DATA_TYPE_STOKES_IQUV
        observation.output.incoherent_stokes.enabled= false
        observation.output.incoherent_stokes.type=

        # for calibration-pipeline
        dp.output.correlated.enabled= false
        dp.output.correlated.demixing_settings.freq_step=
        dp.output.correlated.demixing_settings.time_step=
        dp.output.instrument_model.enabled= false

        # for longbaseline-pipeline
        dp.output.longbaseline.enabled= false
        dp.output.longbaseline.subband_groups_per_ms=
        dp.output.longbaseline.subbands_per_subband_group=

        # for pulsar-pipeline
        dp.output.pulsar.enabled= false

        # for image-pipeline
        dp.output.skyimage.enabled= false
        dp.output.skyimage.slices_per_image=
        dp.output.skyimage.subbands_per_image=
        """
        self.check_set.import_string(checkset)
        validset = """
        observation.total_data_size=
        observation.total_bandwidth=
        observation.output_files.dp_correlated_uv.nr_files=
        observation.output_files.dp_correlated_uv.file_size=
        observation.output_files.dp_coherent_stokes.nr_files=
        observation.output_files.dp_coherent_stokes.file_size=
        observation.output_files.dp_incoherent_stokes.nr_files=
        observation.output_files.dp_incoherent_stokes.file_size=
        """
        self.valid_set.import_string(valid_response)


    def add_observation_beams(self):
        checkset = """
        observation.nr_beams= 2
        observation.beam[0].nr_subbands= 400
        observation.beam[0].nr_tab_rings= 4
        observation.beam[0].tied_array_beam[0].coherent= true
        observation.beam[0].tied_array_beam[1].coherent= true
        observation.beam[1].nr_subbands= 400
        observation.beam[1].nr_tab_rings= 4
        observation.beam[1].tied_array_beam[0].coherent= true
        observation.beam[1].tied_array_beam[1].coherent= true
        """
        self.check_set.import_string(checkset)
        self.check_set.import_string(checkset)
        validset = """
        observation.total_data_size=
        observation.total_bandwidth=
        observation.output_files.dp_correlated_uv.nr_files=
        observation.output_files.dp_correlated_uv.file_size=
        observation.output_files.dp_coherent_stokes.nr_files=
        observation.output_files.dp_coherent_stokes.file_size=
        observation.output_files.dp_incoherent_stokes.nr_files=
        observation.output_files.dp_incoherent_stokes.file_size=
        """
        self.valid_set.import_string(valid_response)


    def enable_observations_coherent_stokes(self):
        checkset = """
        # coherent_stokes.type can be: DATA_TYPE_XXYY or DATA_TYPE_STOKES_IQUV
        observation.output.coherent_stokes.enabled= true
        observation.output.coherent_stokes.type= DATA_TYPE_XXYY
        observation.output.coherent_stokes.integration_factor= 1
        """
        self.check_set.import_string(checkset)
        self.check_set.import_string(checkset)
        validset = """
        observation.total_data_size=
        observation.total_bandwidth=
        observation.output_files.dp_correlated_uv.nr_files=
        observation.output_files.dp_correlated_uv.file_size=
        observation.output_files.dp_coherent_stokes.nr_files=
        observation.output_files.dp_coherent_stokes.file_size=
        observation.output_files.dp_incoherent_stokes.nr_files=
        observation.output_files.dp_incoherent_stokes.file_size=
        """
        self.valid_set.import_string(valid_response)


    def enable_observations_incoherent_stokes(self):
        checkset = """
        # incoherent_stokes.type can be: DATA_TYPE_STOKES_IQUV
        observation.output.incoherent_stokes.enabled= true
        observation.output.incoherent_stokes.type= DATA_TYPE_STOKES_IQUV
        """
        self.check_set.import_string(checkset)
        self.check_set.import_string(checkset)
        validset = """
        observation.total_data_size=
        observation.total_bandwidth=
        observation.output_files.dp_correlated_uv.nr_files=
        observation.output_files.dp_correlated_uv.file_size=
        observation.output_files.dp_coherent_stokes.nr_files=
        observation.output_files.dp_coherent_stokes.file_size=
        observation.output_files.dp_incoherent_stokes.nr_files=
        observation.output_files.dp_incoherent_stokes.file_size=
        """
        self.valid_set.import_string(valid_response)


    def enabble_flys_eye(self):
        checkset = """
        observation.flys_eye.enabled= true
        """
        self.check_set.import_string(checkset)
        self.check_set.import_string(checkset)
        validset = """
        observation.total_data_size=
        observation.total_bandwidth=
        observation.output_files.dp_correlated_uv.nr_files=
        observation.output_files.dp_correlated_uv.file_size=
        observation.output_files.dp_coherent_stokes.nr_files=
        observation.output_files.dp_coherent_stokes.file_size=
        observation.output_files.dp_incoherent_stokes.nr_files=
        observation.output_files.dp_incoherent_stokes.file_size=
        """
        self.valid_set.import_string(valid_response)

    # for all pipelines
    def enable_calibration_pipeline(self):
        checkset = """
        # for calibration-pipeline
        dp.output.correlated.enabled= true
        dp.output.correlated.demixing_settings.freq_step= 60
        dp.output.correlated.demixing_settings.time_step= 10
        dp.output.instrument_model.enabled= true
        """
        self.check_set.import_string(checkset)
        self.check_set.import_string(checkset)
        validset = """
        calibration_pipeline.total_data_size=
        calibration_pipeline.total_bandwidth=
        calibration_pipeline.dp_correlated_uv.nr_files=
        calibration_pipeline.dp_correlated_uv.file_size=
        calibration_pipeline.dp_instrument_model.nr_files=
        calibration_pipeline.dp_instrument_model.file_size=
        """
        self.valid_set.import_string(valid_response)


    def enable_longbaseline_pipeline(self):
        checkset = """
        # for -pipeline
        dp.output.longbaseline.enabled= true
        dp.output.longbaseline.subband_groups_per_ms= 1
        dp.output.longbaseline.subbands_per_subband_group= 1
        """
        self.check_set.import_string(checkset)
        self.check_set.import_string(checkset)
        validset = """
        longbaseline_pipeline.total_data_size=
        longbaseline_pipeline.total_bandwidth=
        longbaseline_pipeline.dp_correlated_uv.nr_files=
        longbaseline_pipeline.dp_correlated_uv.file_size=
        """
        self.valid_set.import_string(valid_response)


    def enable_pulsar_pipeline(self):
        checkset = """
        # for pulsar-pipeline
        dp.output.pulsar.enabled= true
        """
        self.check_set.import_string(checkset)
        self.check_set.import_string(checkset)
        validset = """
        pulsar_pipeline.total_data_size=
        pulsar_pipeline.total_bandwidth=
        pulsar_pipeline.dp_pulsar.nr_files=
        pulsar_pipeline.dp_pulsar.file_size=
        """
        self.valid_set.import_string(valid_response)


    def enable_image_pipeline(self):
        checkset = """
        # for image-pipeline
        dp.output.skyimage.enabled= true
        dp.output.skyimage.slices_per_image= 1
        dp.output.skyimage.subbands_per_image= 2
        """
        self.check_set.import_string(checkset)
        self.check_set.import_string(checkset)
        validset = """
        image_pipeline.total_data_size=
        image_pipeline.total_bandwidth=
        image_pipeline.dp_sky_image.nr_files=
        image_pipeline.dp_sky_image.file_size=
        """
        self.valid_set.import_string(valid_response)

