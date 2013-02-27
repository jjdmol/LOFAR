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

package  gov.noaa.pmel.util;

import java.io.Serializable;

/**
 * <code>SoTPoint</code> has two coordinates which are of
 * type <code>SoTValue</code>. SoT stands for
 * space or time, but being basically lazy I've abbreviated it.
 *
 * @author Donald Denbo
 * @version $Revision$, $Date$
 * @sgt 2.0
 */
public class SoTPoint implements Serializable, Cloneable {
  /** X coordinate  */
  private SoTValue x_;
  /** Y coordinate  */
  private SoTValue y_;
  /**
   * Default constructor.
   */
  public SoTPoint() {
  }
  /**
   * Construct a <code>SoTPoint</code> from <code>SoTValue</code>s.
   *
   * @param x space or time coordinate
   * @param y space or time coordinate
   */
  public SoTPoint(SoTValue x, SoTValue y) {
    x_ = x;
    y_ = y;
  }
  /**
   * Construct a <code>SoTPoint</code> from <code>double</code>s.
   */
  public SoTPoint(double x, double y) {
    this(new SoTValue.Double(x), new SoTValue.Double(y));
  }
  /**
   * Construct a <code>SoTPoint</code> from a <code>double</code> and
   * a <code>GeoDate</code>.
   */
  public SoTPoint(double x, GeoDate y) {
    this(new SoTValue.Double(x), new SoTValue.Time(y));
  }
  /**
   * @since sgt 3.0
   */
  public SoTPoint(double x, long y) {
    this(new SoTValue.Double(x), new SoTValue.Time(y));
  }
  /**
   * Construct a <code>SoTPoint</code> from a <code>GeoDate</code> and
   * a <code>double</code>.
   */
  public SoTPoint(GeoDate x, double y) {
    this(new SoTValue.Time(x), new SoTValue.Double(y));
  }
  /**
   * @since sgt 3.0
   */
  public SoTPoint(long x, double y) {
    this(new SoTValue.Time(x), new SoTValue.Double(y));
  }
  /**
   * Construct a <code>SoTPoint</code> from a <code>SoTPoint</code>.
   */
  public SoTPoint(SoTPoint pt) {
    this(pt.getX(), pt.getY());
  }
  /**
   * Get x value
   */
  public SoTValue getX() {
    return x_;
  }
  /**
   * Set x value
   * @since sgt 3.0
   */
  public void setX(SoTValue x) {
    x_ = x;
  }
  /**
   * Get y value
   */
  public SoTValue getY() {
    return y_;
  }
  /**
   * Set y value
   * @since sgt 3.0
   */
  public void setY(SoTValue y) {
    y_ = y;
  }
  /**
   * Test for equality.  For equality both x and y values must be
   * equal.
   */
  public boolean equals(SoTPoint stp) {
    return (x_.equals(stp.getX()) &&
            y_.equals(stp.getY()));
  }
  /**
   * Test if x value is time
   */
  public boolean isXTime() {
    return x_.isTime();
  }
  /**
   * Test if y value is time
   */
  public boolean isYTime() {
    return y_.isTime();
  }
  /**
   * Add to point.
   *
   * @since sgt 3.0
   */
   public void add(SoTPoint point) {
     x_.add(point.getX());
     y_.add(point.getY());
   }
   /**
    * Make a copy of the <code>SoTRange</code>.
    * @since sgt 3.0
    */
   public SoTPoint copy() {
     try {
       return (SoTPoint)clone();
     } catch (CloneNotSupportedException e) {
       return null;
     }
   }
  /**
   * Convert <code>SoTPoint</code> to a default string
   *
   * @return string representation of the SoTPoint.
   */
  public String toString() {
    return new String("(" + x_ + ", " + y_ + ")");
  }
}
