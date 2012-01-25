
def expected_fluxes_in_fov(conn, ra_c, decl_c, fov_radius, assoc_theta,
                           output_path, storespectraplots = False):
    """
    Muck skymodel creation: Only creates the file on output_path.
    The file is filled with the supplied assoc_theta
    """
    
    # If needed throw exception
    if output_path == "except":
        raise Exception("error")
    
    
    fp = open(output_path, "w")
    fp.write(str(assoc_theta))
    fp.write("\nbbs.skymodel.test")
    fp.close()
