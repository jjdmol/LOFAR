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
package gov.noaa.pmel.sgt.swing;

import java.text.DecimalFormat;

import gov.noaa.pmel.util.SoTPoint;
import gov.noaa.pmel.util.SoTValue;

/**
 * <code>ValueIconFormat</code> is used to create the value string for
 * <code>ValueIcon</code>. This class can be extended to create more
 * sophisticated formatting.  For example, handling the modulo 360 of
 * longitude coordinates.
 *
 * @author Donald Denbo
 * @version $Revision$, $Date$
 * @since 2.0
 * @see DecimalFormat
 * @see ValueIcon
 */
public class ValueIconFormat {
  protected DecimalFormat xfrm_;
  protected DecimalFormat yfrm_;
  protected String tfrm_ = "yyyy-MM-dd HH:mm:ss z";
  /**
   * Construct <code>ValueIconFormat</code> from x and y coordinate
   * <code>DeciamalFormat</code>s.
   */
  public ValueIconFormat(String xfrmt, String yfrmt) {
    xfrm_ = new DecimalFormat(xfrmt);
    yfrm_ = new DecimalFormat(yfrmt);
  }
  /**
   * Format a string using <code>DecimalFormat</code> for x and y
   * coordinates.
   */
  public String format(double x, double y) {
    return "(" + xfrm_.format(x) + ", " + yfrm_.format(y) + ")";
  }
  /**
   * Define the time format.
   *
   * @since 3.0
   */
   public void setTimeFormat(String tfrmt) {
    tfrm_ = tfrmt;
   }
  /**
   * Format a string using <code>DecimalFormat</code> for x and y
   * coordinates or <code>GeoDate</code> formatting for time.
   *
   * @since 3.0
   */
  public String format(SoTPoint pt) {
    StringBuffer sbuf = new StringBuffer("(");
    if(pt.isXTime()) {
      sbuf.append(pt.getX().getGeoDate().toString(tfrm_));
    } else {
      sbuf.append(xfrm_.format(((SoTValue.Double)pt.getX()).getValue()));
    }
    sbuf.append(", ");
    if(pt.isYTime()) {
      sbuf.append(pt.getY().getGeoDate().toString(tfrm_));
    } else {
      sbuf.append(yfrm_.format(((SoTValue.Double)pt.getY()).getValue()));
    }
    sbuf.append(")");
    return sbuf.toString();
  }
}
