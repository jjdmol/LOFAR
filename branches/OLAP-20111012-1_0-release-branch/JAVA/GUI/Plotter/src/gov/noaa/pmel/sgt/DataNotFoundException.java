/*
 * $Id$
 *
 * This software is provided by NOAA for full, free and open release.  It is
 * understood by the recipient/user that NOAA assumes no liability for any
 * errors contained in the code.  Although this software is released without
 * conditions or restrictions in its use, it is expected that appropriate
 * credit be given to its author and to the National Oceanic and Atmospheric
 * Administration should the software be included by the recipient as an
 * element in other product development.
 */

package  gov.noaa.pmel.sgt;

/**
 * Graph could not be produced because no data has been assigned.
 *
 * @author Donald Denbo
 * @version $Revision$, $Date$
 * @since 2.0
 */
public class DataNotFoundException extends SGException {
  public DataNotFoundException() {
    super();
}
  public DataNotFoundException(String s) {
    super(s);
  }
}
