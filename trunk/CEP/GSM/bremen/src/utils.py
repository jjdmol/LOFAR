#!/usr/bin/python
import sys
import healpy
from math import radians, cos
try:
    # Try loading LOFAR parset support, fallback to ConfigObj.
    from lofar.parameterset import parameterset
    LOFAR_PARAMETERSET = True
except ImportError:
    from configobj import ConfigObj
    LOFAR_PARAMETERSET = False


def load_parameters(filename):
    """
    Load parameters from file and return them as hash.
    """
    if LOFAR_PARAMETERSET:
        data = parameterset(filename).dict()
    else:
        data = ConfigObj(filename, raise_errors=True, file_error=True)
    return data


def get_pixels(centr_ra, centr_decl, fov_radius):
    """
    Get a list of HEALPIX zones that contain a given image.
    """
    vector = healpy.ang2vec(radians(90.0 - centr_decl),
                            radians(centr_ra))
    pixels = healpy.query_disc(32, vector, radians(fov_radius),
                               inclusive=True, nest=True)
    return str(pixels.tolist())[1:-1]


def get_image_size(min_decl, max_decl, min_ra, max_ra, avg_decl, avg_ra):
    """
    >>> get_image_size(1.0, 3.0, 1.0, 3.0, 2.0, 2.0)
    (1.0, 2.0, 2.0)
    >>> get_image_size(-4.0, 4.0, 1.0, 359.0, 0.0, 359.8)
    (4.0, 0.0, 0.0)
    """
    if max_ra - min_ra > 250.0:
        # Field across zero-ra. Has to be shifted.
        # E.g. min = 0.1 max = 359.7 avg = 359.9
        # transfers to:
        # min = -0.3 max = 0.1 avg = -0.1
        min_ra, max_ra = max_ra - 360.0, min_ra
        avg_ra = 0.5 * (max_ra + min_ra)
    min_ra = min_ra * cos(avg_decl)
    max_ra = max_ra * cos(avg_decl)
    return max([avg_decl - min_decl, max_decl - avg_decl,
                avg_ra * cos(avg_decl) - min_ra,
                max_ra - avg_ra * cos(avg_decl)]), \
                avg_decl, avg_ra

def raise_with_message(exc, message):
    raise type(exc), type(exc)('%s %s' % (exc.message, message)), \
          sys.exc_info()[2]
