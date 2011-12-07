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

package gov.noaa.pmel.util;

import java.util.StringTokenizer;

/**
 * Uility methods for accessing Java System information.
 *
 * @author Donald Denbo
 * @version $Revision$, $Date$
 * @since 3.0
 **/
public class EPICSystem {

  public EPICSystem() {
  }

  /**
   * Get the major java version.  If 1.4.2, returns 1.
   * @return java version
   */
  static public int getJavaMajorVersion() {
    StringTokenizer st = new StringTokenizer(System.getProperty("java.version"),
        ".", false);
    return Integer.parseInt(st.nextToken());
  }
  /**
   * Get the minor java version.  If 1.4.2, returns 2.
   * @return java minor version
   */
 static public int getJavaMinorVersion() {
   StringTokenizer st = new StringTokenizer(System.getProperty("java.version"),
       ".", false);
   st.nextToken();
   return Integer.parseInt(st.nextToken());
  }
}