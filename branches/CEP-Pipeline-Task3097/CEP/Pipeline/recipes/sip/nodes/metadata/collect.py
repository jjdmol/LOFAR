#                                                       LOFAR PIPELINE FRAMEWORK
#
#                           Collect meta-data of Standard Imaging data prouducts
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

from lofarpipe.support.utilities import disk_usage

import pyrap.tables
import pyrap.images

def uv(ms):
    """
    Collect the UV metadata from the given input Measurement Set. 
    Return the results in a dict.
    """
    main = pyrap.tables.table(ms)
    spw = pyrap.tables.table(main.getkeyword('SPECTRAL_WINDOW'))
    exposure = main.getcell('EXPOSURE', 0)
    startTime = main.getcell('TIME', 0) - 0.5 * exposure
    endTime = main.getcell('TIME', main.nrows() - 1) + 0.5 * exposure
    data = {
        'size' : disk_usage(ms),
        'fileFormat' : "AIPS++/CASA",
        'startTime' : startTime,
        'duration' : endTime - startTime
        'integrationInterval' : exposure,
        'centralFrequency' : spw.getcell('REF_FREQUENCY', 0),
        'channelWidth' : spw.getcell('RESOLUTION', 0)[0],
        'channelsPerSubband' : spw.getcell('NUM_CHAN', 0)
    }
    return data
    

def _pointing():
    data = {
        'equinox' : "??",
        'coordType' : "??",
        'coordVal0' : "??",
        'coordVal1' : "??",
    }
    return data

    
def _direction_coordinate(direction):
    data = {
#        'number' : 1,
        'nrDirectionAxis' : len(direction.get_axes()),
        'PC0_0' : direction.dict()['pc'][0][0],
        'PC0_1' : direction.dict()['pc'][0][1],
        'PC1_0' : direction.dict()['pc'][1][0],
        'PC1_1' : direction.dict()['pc'][1][1],
        'equinox' : direction.get_frame(),
        'raDecSystem' : "FK5",                   ### CORRECT ??? ###
        'projection' : direction.get_projection(),
        'projectionParameters' : direction.dict()['projection_parameters'],
        'longitudePole' : direction.dict()['longpole'],
        'latitudePole' : direction.dict()['latpole'],
        'directionAxis' : [
            {
#                    'number' : n,
                'name' : direction.get_axes()[n],
                'units' : direction.get_unit()[n],
#                    'length' : "??",
                'increment' : direction.get_increment()[n],
                'referencePixel' : direction.get_referencepixel()[n],
                'referenceValue' : direction.get_referencevalue()[n]
            }
#                for n in range(nrDirectionAxis)
            for n in range(len(direction.get_axes()))
        ]
    }
    return data


def _spectral_coordinate(spectral):
    data = {
#        'number' : 1,
        'spectralQuantityType' : "VelocityRadio",  ### CORRECT ??? ###
        'spectralQuantityValue' : "??"
    }
    _axis = {
#        'number' : 1,
        'name' : spectral.dict()['name'],
        'units' : spectral.get_unit(),
#        'length' : "??"
    }
    if spectral.dict()['wcs']:      ## Linear if coord.spectral.wcs exists 
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


def _polarization_coordinate(stokes):
    data = {
#        'number' : 0,
        'polarizationType' : stokes.get_stokes()
        'PolarizationTabularAxis' : {
#            'number' : 0,
            'name' : stokes.get_axes()[0],
            'units' : "",
            'length' : len(stokes.get_stokes())
        }
    }
    return data


def image(img):
    """
    Collect the image metadata from the given input CASA image.
    Return the results in a dict.
    """
    image = pyrap.images.image(img)
    coord = image.coordinates()
    direction = coord.get_coordinate('direction')
    spectral = coord.get_coordinate('spectral')
    stokes = coord.get_coordinate('stokes')
    data = {
        'size' : disk_usage(img),
        'fileFormat' : "AIPS++/CASA",
        'numberOfAxes' : len(coord.get_axes()),
        'locationFrame' : spectral.get_frame(),      ### CORRECT ??? ###
        'timeFrame' : coord.get_obsdate()['refer'],  ### CORRECT ??? ###
        'Pointing' : _pointing(),
        'DirectionCoordinate' : _direction_coordinate(direction),
        'SpectralCoordinate' : _spectral_coordinate(spectral),
        'PolarizationCoordinate' : _polarization_coordinate(stokes)
    }
    return data
    
