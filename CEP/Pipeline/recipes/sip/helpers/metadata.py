#                                                         LOFAR IMAGING PIPELINE
#
#                                                Data product meta-data handling
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

"""
This module contains helper classes and methods for the handling of meta-data
of data products produced by the LOFAR Standard Imaging Pipeline. Meta-data is
typically retrieved from the data products itself. Internally, it is stored as
a nested dict. The `to_parset()` can be used to convert this dict into a LOFAR 
parameterset that can be handled by MAC/SAS.
"""

from lofarpipe.support.utilities import disk_usage
from lofar.parameterset import parameterset

import pyrap.tables
import pyrap.images
import os
import sys
import socket


def to_parset(data, prefix=''):
    """
    Convert the data in the variable `data` to a LOFAR parameterset. Values
    may contain vectors (python lists) or records (python dicts) of scalars.
    Deeper nested structures must be unraveled in separate key/value pairs,
    where the name of the nested value is moved into the key. Keys for
    vector values will get an indexed attached to their name.
    
    For example, the dictionary entry
        'vec_rec' : [{1:'a', 2:'b'}, {3:'c'}]
    will be converted to the following parameterset key/value pairs
        vec_rec[0]={1: 'a', 2: 'b'}
        vec_rec[1]={3: 'c'}
    And, the dictionary entry
        'rec_vec' : {'a':[1, 2], 'b':[3]}
    will be converted to
        rec_vec.a=[1, 2]
        rec_vec.b=[3]
    """
    result = parameterset()
    if isinstance(data, dict):
        for key, value in data.iteritems():
            fullkey = prefix + '.' + key if prefix else key
            if isinstance(value, dict):
                if any(isinstance(v, dict) or isinstance(v, list) 
                    for v in value.values()):
                    result.adoptCollection(to_parset(value, fullkey))
                else:
                    result.replace(fullkey, str(value))
            elif isinstance(value, list):
                if any(isinstance(v, dict) or isinstance(v, list) 
                    for v in value):
                    result.adoptCollection(to_parset(value, fullkey))
                else:
                    result.replace(fullkey, str(value))
            else:
                result.replace(fullkey, str(value))
    elif isinstance(data, list):
        for index, value in enumerate(data):
            fullkey = prefix + '[%d]' % index
            if isinstance(value, dict):
                if any(isinstance(v, dict) or isinstance(v, list) 
                    for v in value.values()):
                    result.adoptCollection(to_parset(value, fullkey))
                else:
                    result.replace(fullkey, str(value))
            elif isinstance(value, list):
                if any(isinstance(v, dict) or isinstance(v, list) 
                    for v in value):
                    result.adoptCollection(to_parset(value, fullkey))
                else:
                    result.replace(fullkey, str(value))
            else:
                result.replace(fullkey, str(value))
    return result



class DataProduct(object):
    """
    Base class for data product metadata.
    """
    def __init__(self):
        self._data = {
            'size' : 0,
            'fileFormat' : "",
            'filename' : "",
            'location' : "",
            'percentageWritten' : 0
        }


    def data(self):
        """Return the current metadata."""
        return self._data
        
        
    def as_parameterset(self):
        """Return the current data product into a LOFAR parameterset."""
        return to_parset(self._data)


    def collect(self, filename):
        """
        Collect the metadata that is part of the DataProduct base class.
        """
        self._data.update({
            'size' : disk_usage(filename),
            'fileFormat' : "AIPS++/CASA",
            'filename' : os.path.basename(filename),
            'location' : "%s:%s" % (
                socket.gethostname(), os.path.abspath(os.path.dirname(filename))
            )
        })
        # Set percentageWritten to an arbitrary 50, if filesize is non-zero.
        if self._data['size'] > 0:
            self._data['percentageWritten'] = 50



class Correlated(DataProduct):
    """
    Class representing the metadata associated with UV-correlated data.
    The optional argument `filename` is the name of the Measurement Set.
    """
    def __init__(self, filename=None):
        super(Correlated, self).__init__()
        self._data.update({
            'startTime' : "not-a-datetime",
            'duration' : 0.0,
            'integrationInterval' : 0.0,
            'centralFrequency' : 0.0,
            'channelWidth' : 0.0,
            'channelsPerSubband' : 0,
            'subband' : 0,
            'stationSubband' : 0
        })
        if filename: 
            self.collect(filename)


    def collect(self, filename):
        """
        Collect UV-correlated metadata from the Measurement Set `filename`.
        """
        super(Correlated, self).collect(filename)
        try:
            main = pyrap.tables.table(filename, ack=False)
            spw = pyrap.tables.table(main.getkeyword('SPECTRAL_WINDOW'), ack=False)
            exposure = main.getcell('EXPOSURE', 0)
            startTime = main.getcell('TIME', 0) - 0.5 * exposure
            endTime = main.getcell('TIME', main.nrows() - 1) + 0.5 * exposure
            # This is an ugly way to get the start time in the format that
            # OTDB/MoM expects, yyyy-mm-ddTHH:MM:SS, because it depends on the
            # output string format of 'ctod'.
            startTimeString = (
                pyrap.tables.taql('calc ctod($startTime s)')[0]
                    .replace('/', '-', 2).replace('/', 'T')
            )
            self._data.update({
                'percentageWritten' : 100,
                'startTime' : startTimeString,
                'duration' : endTime - startTime,
                'integrationInterval' : exposure,
                'centralFrequency' : spw.getcell('REF_FREQUENCY', 0),
                'channelWidth' : spw.getcell('RESOLUTION', 0)[0],
                'channelsPerSubband' : spw.getcell('NUM_CHAN', 0),
                # Assume subband name has format 'SB-nn'
                'subband' : int(spw.getcell('NAME', 0)[3:]),
                'stationSubband' : 0         ### NOT CORRECT! ###
            })
        except Exception, error:
            print >> sys.stderr, (
                "%s: %s\n\twhile processing file %s" % 
                (type(error).__name__, error, filename)
            )



class InstrumentModel(DataProduct):
    """
    Class representing the metadata associated with an instrument model.
    """
    def __init__(self, filename=None):
        """
        Constructor. The optional argument `filename` is the name of the
        Measurement Set containing the instrument model.
        """
        DataProduct.__init__(self)
        if filename: 
            self.collect(filename)


    def collect(self, filename):
        """
        Collect instrument model metadata from the Measurement Set `filename`.
        """
        super(InstrumentModel, self).collect(filename)
        if self._data['size'] > 0:
            self._data['percentageWritten'] = 100



class SkyImage(DataProduct):
    """
    Class representing the metadata associated with a sky image.
    """
    def __init__(self, filename=None):
        """
        Constructor. The optional argument `filename` is the name of the
        CASA Image containing the sky image.
        """
        DataProduct.__init__(self)
        self._data.update({
            'numberOfAxes' : 0,
            'nrOfDirectionCoordinates' : 0,
            'nrOfSpectralCoordinates' : 0,
            'nrOfPolarizationCoordinates' : 0,
            'coordinateTypes' : [],
            'locationFrame' : "GEOCENTER",
            'timeFrame' : "",
            'restoringBeamMajorUnit' : "arcsec",
            'restoringBeamMajorValue' : 0,
            'restoringBeamMinorUnit' : "arcsec",
            'restoringBeamMinorValue' : 0,
            'rmsNoiseValue' : 0,
            'rmsNoiseUnit' : ""
        })
        if filename: 
            self.collect(filename)

        
    def collect(self, filename):
        """
        Collect sky image metadata from the CASA Image `filename`.
        """
        super(SkyImage, self).collect(filename)    
        try:
            image = pyrap.images.image(filename)
            coord = image.coordinates()
            beaminfo = image.imageinfo()['restoringbeam']
            self._data.update({
                'numberOfAxes' : image.ndim(),
                'coordinateTypes' : coord._names,
                'timeFrame' : coord.get_obsdate()['refer'],
                'restoringBeamMajorUnit' : beaminfo['major']['unit'],
                'restoringBeamMajorValue' : beaminfo['major']['value'],
                'restoringBeamMinorUnit' : beaminfo['minor']['unit'],
                'restoringBeamMinorValue' : beaminfo['minor']['value'],
                'rmsNoiseValue' : image.attrgetrow(
                    'LOFAR_QUALITY', 'QUALITY_MEASURE', 'RMS_NOISE_I'
                )['VALUE'],
                'rmsNoiseUnit' : image.unit()
            })
            if 'direction' in coord._names:
                direction = coord.get_coordinate('direction')
                self._data.update({
                    'nrOfDirectionCoordinates' : 1,
                    'DirectionCoordinate' : self._get_direction_coord(direction)
                })
            if 'spectral' in coord._names:
                spectral = coord.get_coordinate('spectral')
                self._data.update({
                    'locationFrame' : spectral.get_frame(),
                    'nrOfSpectralCoordinates' : 1,
                    'SpectralCoordinate' : self._get_spectral_coord(spectral)
                })
            if 'stokes' in coord._names:
                stokes = coord.get_coordinate('stokes')
                self._data.update({
                    'nrOfPolarizationCoordinates' : 1,
                    'PolarizationCoordinate' : self._get_stokes_coord(stokes)                
                })
            self._data.update({
                'Pointing' : self._get_pointing(image),
                'percentageWritten' : 100
            })
        except Exception, error:
            print >> sys.stderr, (
                "%s: %s\n\twhile processing file %s" % 
                (type(error).__name__, error, filename)
            )


    @staticmethod
    def _get_pointing(image):
        angles = list(image.attrget('LOFAR_FIELD', 'PHASE_DIR', 0)[0])
        data = {
            'equinox' : image.attrgetmeas('LOFAR_FIELD', 'PHASE_DIR')[1],
            'coordType' : "RA-DEC",     ### Correct? ###
            'angle1' : angles[0],
            'angle2' : angles[1]
        }
        return data


    @staticmethod
    def _get_direction_coord(direction):
        data = {
            'nrDirectionLinearAxis' : len(direction.get_axes()),
            'PC0_0' : direction._coord['pc'][0][0],
            'PC0_1' : direction._coord['pc'][0][1],
            'PC1_0' : direction._coord['pc'][1][0],
            'PC1_1' : direction._coord['pc'][1][1],
            'equinox' : direction.get_frame(),
            'raDecSystem' : "ICRS",
            'projection' : direction.get_projection(),
            'projectionParameters' : list(direction._coord['projection_parameters']),
            'longitudePole' : direction._coord['longpole'],
            'latitudePole' : direction._coord['latpole'],
            'DirectionLinearAxis' : [
                {
                    'name' : direction.get_axes()[n],
                    'units' : direction.get_unit()[n],
                    'length' : direction.get_axis_size(n),
                    'increment' : direction.get_increment()[n],
                    'referencePixel' : direction.get_referencepixel()[n],
                    'referenceValue' : direction.get_referencevalue()[n]
                }
                for n in range(len(direction.get_axes()))
            ]
        }
        return data


    @staticmethod
    def _get_spectral_coord(spectral):
        data = {
            'spectralQuantityType' : "Frequency",
            'spectralQuantityValue' : spectral.get_referencevalue()
        }
        _axis = {
            'name' : spectral._coord['name'],
            'units' : spectral.get_unit(),
            'length' : spectral.get_axis_size()
        }
        # Assume spectral linear axis if spectral._coord['wcs'] exists 
        if spectral._coord['wcs']:
            _axis.update({
                'increment' : spectral.get_increment(),
                'referencePixel' : spectral.get_referencepixel(),
                'referenceValue' : spectral.get_referencevalue()
            })
            data.update({
                'spectralAxisType' : "Linear",
                'SpectralLinearAxis' : _axis
            })
        else:
            data.update({
                'spectralAxisType' : "Tabular",
                'SpectralTabularAxis' : _axis
            })
        return data


    @staticmethod
    def _get_stokes_coord(stokes):
        data = {
            'polarizationType' : stokes.get_stokes(),
            'PolarizationTabularAxis' : {
                'name' : stokes.get_axes()[0],
                'units' : "",
                'length' : len(stokes.get_stokes())
            }
        }
        return data



if __name__ == "__main__":
    Correlated('sample_uv.MS').as_parameterset().writeFile('Correlated.parset')
    InstrumentModel('sample_inst.INST').as_parameterset().writeFile('InstrumentModel.parset')
    SkyImage('sample_sky.IM').as_parameterset().writeFile('SkyImage_IM.parset')
    SkyImage('sample_sky.h5').as_parameterset().writeFile('SkyImage_h5.parset')

