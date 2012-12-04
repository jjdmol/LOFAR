import numpy
import constants

def notice(msg):
    print "\033[1;32m%s\033[m" % msg

def warning(msg):
    print "\033[1;33mwarning: %s\033[m" % msg

def error(msg):
    print "\033[1;31merror: %s\033[m" % msg

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
