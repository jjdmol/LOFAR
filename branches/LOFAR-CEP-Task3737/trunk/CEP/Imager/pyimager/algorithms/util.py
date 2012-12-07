import numpy
import constants

import pyrap.images
import matplotlib.pyplot
import datetime

def now():
    _now = datetime.datetime.now()
    return _now.strftime("%Y/%m/%d/%H:%M:%S")

def notice(msg):
    print "\033[94m[%s] %s\033[m" % (now(), msg)

def warning(msg):
    print "\033[93m[%s] warning: %s\033[m" % (now(), msg)

def error(msg):
    print "\033[91m[%s] error: %s\033[m" % (now(), msg)

def store_image(name, coordinates, image):
    assert(len(image.shape) <= 4)
    shape = [1 for i in range(4)]
    shape[(4 - len(image.shape)):] = image.shape
    im = pyrap.images.image(name, shape=shape, coordsys=coordinates)
    im.putdata(image)

def show_image(data, title=None):
    """Create a figure with 2 x 2 subplots that show the four correlation planes
    of the image, on the same scale.

    Keyword arguments:
    data -- A 4 x N x M array of values to be plotted.
    title -- An optional title to be put at the top of the combined plot.
    """
    fig, axes = matplotlib.pyplot.subplots(nrows=2, ncols=2)
    if title is not None:
        fig.suptitle(title, fontsize=14)

    vmin = numpy.min(data)
    vmax = numpy.max(data)
    for k, ax in zip(range(4), axes.flat):
        __im = ax.imshow(data[k,:,:], origin="lower", interpolation="nearest", \
            cmap="bone", vmin=vmin, vmax=vmax) # aspect="auto")
        __im.axes.get_xaxis().set_visible(False)
        __im.axes.get_yaxis().set_visible(False)

    fig.subplots_adjust(right=0.70, wspace=0.0, hspace=0.0)
    cax = fig.add_axes([0.75, 0.1, 0.03, 0.8])
    cbr = fig.colorbar(__im, cax=cax)
    cbr.set_clim(vmin, vmax)

def full_width_half_max(diameter, freq):
    """Return an estimate of the full width half maximum of the beam given the
    station diameter and the frequency of interest."""
    return constants.speed_of_light / (freq * diameter)

def image_configuration(image_diameter, freq, max_baseline):
    """Determine the number of pixels and the increments on the image plane
    required to make an image at a resolution of about three pixels per beam,
    given the image diameter (rad), station diameter (m), maximal baseline
    length (m), and the frequency (Hz)."""
    # TODO: Should curvature be considered in this estimate (for large FOV)?
    wl = constants.speed_of_light / freq
    delta = wl / (3.0 * max_baseline)

    pixels = 2 * int(image_diameter / (2.0 * delta))
    return (pixels, delta)
