//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////

package org.astron.basesim;

/**
 * Title:        Visual LofarSim
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      Astron
 * @author Robbert Dam
 * @version 1.0
 */

import java.awt.*;
import java.awt.geom.*;

/**
 * This class represents the visual representation of a connection between two
 * Graph objects.
 *
 * Graph are connected as shown below
 * <code>graphOut->graphIn</code>
 *
 * Other types of Connections should inherit from this class.
 */
public class Connection extends java.lang.Object {

  static float dash1[] = {6.0f};
  static float dash2[] = {3.0f};

  private final static int CURVE_X_CONSTANT = 250;
  private final static int CURVE_Y_CONSTANT = 100;
  private final static int LINE_CONSTANT = 10;
  protected final static Stroke SIMPLE_STROKE = new BasicStroke(1);
  protected final static Stroke THICK_STROKE = new BasicStroke(2);
  protected final static Stroke SIMPLE_DASH_STROKE =
    new BasicStroke(1,BasicStroke.CAP_SQUARE,
                      BasicStroke.JOIN_MITER,
                      1,dash1,0.0f);
  protected final static Stroke THICK_DASH_STROKE =
    new BasicStroke(2,BasicStroke.CAP_SQUARE,
                      BasicStroke.JOIN_MITER,
                      1,dash1,0.0f);
  protected final static Stroke SIMPLE_DOTTED_STROKE =
    new BasicStroke(1,BasicStroke.CAP_SQUARE,
                      BasicStroke.JOIN_MITER,
                      1,dash2,1.0f);
  protected final static Stroke THICK_DOTTED_STROKE =
    new BasicStroke(2,BasicStroke.CAP_SQUARE,
                      BasicStroke.JOIN_MITER,
                      1,dash2,5.0f);

  /** Graph with outgoing connection (unidirectional stream) */
  public Graph graphOut;
  /** Graph with incoming connection */
  public Graph graphIn;
  private boolean visible;
  /** Only valid when _coordinateSet = true. */
  private int linLength;
  protected Color color, selColor;
  protected Stroke stroke, selStroke;
  private boolean selected;

  /** Create a connection. */
  public Connection() {
    graphOut = null;
    graphIn = null;
    init();
  }

  /** Create a connection between the two specified graphs. The connection
   *  is unidirectional: graphOutput->graphInput */
  public Connection(Graph graphOutput, Graph graphInput) {
    graphIn = graphInput;
    graphOut = graphOutput;
    init();
  }

  private void init() {
    visible = true;
    selected = false;

    // set color for this basic connection
    color = Color.black;
    selColor = Color.red;

    // set stroke shapes for this basic connection
    stroke = SIMPLE_STROKE;
    selStroke = THICK_STROKE;
  }

  /** Set the visibility of this connection. */
  public void setVisible (boolean visible) { this.visible = visible; }
  /** Set if this connection is selected */
  public void setSelected(boolean selected) { this.selected = selected; }
  /** Set the basic drawing color */
  public void setColor (Color color) { this.color = color; }
  /** Get the basic drawing color */
  public Color getColor () { return color; }
  /** Set the color of this connection when selected */
  public void setColorSelected (Color color) { this.selColor = color; }
  /** Get the color of this line when selected */
  public Color getColorSelected () { return selColor; }
  /** Set the stroke used to draw this line */
  public void setStroke (Stroke stroke) { this.stroke = stroke; }
  /** Get the stroke used to draw this line */
  public Stroke getStroke () { return stroke; }
  /** Set the stroke used to draw a selected line */
  public void setStrokeSelected (Stroke stroke) { this.selStroke = stroke; }
  /** Get the stroke used to select this line */
  public Stroke getStrokeSelected () { return selStroke; }
  /** Paint this connection */
  public void paint (Graphics g)
  {
    Graphics2D g2 = (Graphics2D)g;
    Color color = null;
    Stroke stroke = null;
    if (selected) {
      color = selColor;
      stroke = selStroke;
    } else {
      color = this.color;
      stroke = this.stroke;
    }
    Point inP, outP;
    if (!visible) return;
    g2.setPaint(color);
    inP = graphIn.getConnectionPoint(true,this);
    outP = graphOut.getConnectionPoint(false,this);
    if ( (inP == null) || (outP == null) ) return;
    if (inP.x >= outP.x) {
      double square = square(inP,outP);
      int minLength = (int)(graphIn.getZoomLevel()*LINE_CONSTANT);
      int maxLength = (int)((inP.x-outP.x)/2)-minLength;
      linLength = (int)((((0.5*Math.PI-square)/Math.PI)*maxLength+minLength));
      ((Graphics2D)g).setStroke(stroke);
      g.drawLine(outP.x,outP.y,outP.x+linLength,outP.y);
      g.drawLine(outP.x+linLength,outP.y,inP.x-linLength,inP.y);
      g.drawLine(inP.x-linLength,inP.y,inP.x,inP.y);
    } else {
      // loopback
      Point ctrlPIn = new Point(inP);
      Point ctrlPOut = new Point(outP);
      if (ctrlPIn.y < (ctrlPOut.y-graphIn.getHeight())) {
        ctrlPIn.y += (int)(graphIn.getZoomLevel()*CURVE_Y_CONSTANT);
        ctrlPOut.y -= (int)(graphIn.getZoomLevel()*CURVE_Y_CONSTANT);
      } else if (ctrlPIn.y > (ctrlPOut.y+graphIn.getHeight())) {
        ctrlPIn.y -= (int)(graphIn.getZoomLevel()*CURVE_Y_CONSTANT);
        ctrlPOut.y += (int)(graphIn.getZoomLevel()*CURVE_Y_CONSTANT);
      } else {
        ctrlPIn.y -= (int)(graphIn.getZoomLevel()*CURVE_Y_CONSTANT);
        ctrlPOut.y -= (int)(graphIn.getZoomLevel()*CURVE_Y_CONSTANT);
      }
      ctrlPIn.x -= (int)(graphIn.getZoomLevel()*CURVE_X_CONSTANT);
      ctrlPOut.x += (int)(graphIn.getZoomLevel()*CURVE_X_CONSTANT);
      CubicCurve2D.Float curve =
        new CubicCurve2D.Float(inP.x,inP.y,ctrlPIn.x,ctrlPIn.y,
                               ctrlPOut.x,ctrlPOut.y,outP.x,outP.y);
      ((Graphics2D)g).draw(curve);
    }
  }

  /** Helper function used by paint() */
  private double square(Point p1, Point p2) {
    int dx=p1.x-p2.x;
    int dy=p1.y-p2.y;
    if (dx<0) dx*=-1;
    if (dy<0) dy*=-1;
    if (dx == 0) {
      return 0.5*Math.PI;
    } else {
      return Math.atan(dy/dx);
    }
  }
}
