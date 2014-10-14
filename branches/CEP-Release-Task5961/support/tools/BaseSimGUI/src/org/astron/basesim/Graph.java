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
 * Title:        Class Graph
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      Astron
 * @author Robbert Dam
 * @version 1.0
 */

import java.awt.*;
import java.awt.geom.*;
import java.util.*;
import org.astron.util.*;
import javax.swing.*;
import kiwi.util.Config;

 /** The Graph class represents a visual object that can be part of a
  *  composite. A Graph is visualized as a rectangle with rounded corners.
  *  A Graph can display its name, it can be selected and can be rescaled
  *  using a specified zoom level. In addition, a Graph can be connected
  *  to other Graph. A Graph has input and output connection points which
  *  exact locations can be asked to a Graph object.
  *
  *  The Graph class is the base class for all graphical objects.
  */
public class Graph {

  /** Spacing between selection markers and object at a 100% zoom level */
  private final static int SELECT_MARK_SPACING = 1;
  /** Size of black rectangle in selection marker */
  private final static int SELECT_MARK_SIZE = 5;

  /** Followup number for name generation (when Graph name is not specified) */
  static int followUp = 0;

  static int SIDE_WEST = 0;
  static int SIDE_EAST = 1;
  static int SIDE_SOUTH = 2;
  static int SIDE_NORTH = 3;

  /** A multiplier for the relative dimensions */
  private double _zoomLevel;
  /** The current location of this Graph relative to its owner */
  protected Point loc;
  /** The location of the Graph at a 100% zoom level */
  private Point _baseLoc;
  /** The height of this Graph at a 100% zoom level */
  private int _relHeight;
  /** The width of this Graph at a 100% zoom level */
  private int _relWidth;
  /** The current height of this Graph */
  protected int height;
  /** The current width of this Graph */
  protected int width;
  /** The size of the rounding of the basic Graph visualization */
  private int _roundSize;
  /** The size of roundings of this Graph at a 100% zoom level */
  private int _relRoundSize;
  /** Background color */
  private Paint _bkColor;
  /** Line color (foreground color) */
  private Paint _lineColor;
  /** Selection color (may be deprecated in the future) */
  private Paint _selColor;
  /** Is this Graph selected? */
  private boolean _selected;
  /** Is this Graph visible? */
  private int _visible;
  /** Are the borders of this Graph visible? */
  private boolean _borderVisible;
  /** The name of this Graph */
  private String _name;
  /** Should the Graphs name be displayed? */
  private boolean _nameVisible;
  /** The owner of this Graph. Can be null. */
  private GraphComposite _owner;
  /** Dynamic array of input connections */
  protected ArrayList inConn;
  /** Dynamic array of output connectiosn */
  protected ArrayList outConn;
  /** Counter for connection point retreival functions */
  private int _inFollowUp = 0;
  /** Counter for connection point retreival functions */
  private int _outFollowUp = 0;
  /** The level in the composite tree. -1 means 'not yet determined'. */
  private int _treeLevel = 0;
  /** The GraphManager for this Graph object */
  protected GraphManager manager;
  /** Part of the Graph tree can be drawn. Is this Graph the graphical root? */
  private boolean _graphicalRoot = false;

  public Graph ()
  {
    _relHeight = 0;
    _relWidth =0 ;
    loc = new Point(0,0);
    _baseLoc = new Point(0,0);
    _name = "Graph"+followUp++;
    defaultSettings();
  }

  /** Create a Graph on a specified location with no dimensions */
  public Graph(GraphManager man, int x, int y, String name) {
    _relHeight = 0;
    _relWidth =0 ;
    loc = new Point(x,y);
    _baseLoc = new Point(x,y);
    if (name != null) _name = name; else _name = "";
    manager = man;
    defaultSettings();
  }

  /** Create a Graph with specified location and dimensions */
  public Graph(GraphManager man, int x, int y, int width, int height, String name) {
    loc = new Point(x,y);
    _baseLoc = new Point(x,y);
    _relWidth = width;
    _relHeight = height;
    if (name != null) _name = name; else _name = "";
    manager = man;
    defaultSettings();
  }

  private void defaultSettings ()
  {
    _owner = null;
    _borderVisible = true;
    _visible = 0;
    width = 0;
    height = 0;
    _borderVisible = true;
    inConn = new ArrayList();
    outConn = new ArrayList();
    _nameVisible = true;
    _roundSize = 10;
    _zoomLevel = 1;
    height = (int)(_zoomLevel*_relHeight);
    width = (int)(_zoomLevel*_relWidth);
    _relRoundSize = (int)(_zoomLevel*_relRoundSize);
    _selColor = Color.black;
    _lineColor = Color.black;
    _bkColor = Color.white;
    sizeChanged();
  }

  /** Registers the Graph Manager */
  public void setGraphManager(GraphManager manager) { this.manager = manager; }

  public GraphManager getGraphManager() { return manager; }

  protected void transformDimensions() {
    height = (int)(_zoomLevel*_relHeight);
    width = (int)(_zoomLevel*_relWidth);
    loc.x = (int)(_zoomLevel*_baseLoc.x);
    loc.y = (int)(_zoomLevel*_baseLoc.y);
    _roundSize = (int)(_zoomLevel*_relRoundSize);
  }

  /** Recalc the dimensions of this Graph using the given zoom level. */
  public void setZoomLevel (double level)
  {
    _zoomLevel = level;
    transformDimensions();
  }

  /** Return current zoom level*/
  public double getZoomLevel() { return _zoomLevel; }

  /** @deprecated */
  public boolean isGraphicalRoot() { return _graphicalRoot; }
  /** @deprecated */
  public void setGraphicalRoot(boolean graphicalRoot) {
    _graphicalRoot = graphicalRoot;
  }

  /** Enable/disable border visibility */
  public void enableBorder (boolean visible) { _borderVisible = visible; }

  public boolean hasVisibleBorders () { return _borderVisible; }

  // ----- - --- ---- -- - -- -- - ------- - --------- -----   --------- - ---
  // Painting methods.
  // -------- -- --- ---- -- ---------- - - ----------- - --- --  - --- - - --

  /** Invokes <code>paintBorder()</code> and <code>paintGraph()</code>
   *  respectively. */
  public boolean paint ()
  {
    Graphics2D g = manager.getGraphics();
    Point point = getAbsoluteLocation();        // Resolve location
    if (_borderVisible) paintBorder(g,point);
    if (_nameVisible) paintGraph(g,point);
    return true;
  }

  /** Paints the border around this Graph. It will not check if the borders
   *  are visible! This task is left to <code>paint()</code>. It also
   *  fills the background of the Graph with the background color.
   *
   *  @param    g     Graphics context on which to draw
   *  @param    abs   Absolute location of upper-left corner of this Graph */
  protected void paintBorder(Graphics2D g, Point abs) {
    g.setPaint(Color.black);
    g.fillRoundRect(abs.x+2,abs.y+2,width,height,_roundSize,_roundSize);
    g.setPaint(_bkColor);
    g.fillRoundRect(abs.x,abs.y,width,height,_roundSize,_roundSize);
    g.setPaint(_lineColor);
    g.drawRoundRect(abs.x,abs.y,width,height,_roundSize,_roundSize);
    if (_selected) {
      int s = (int)(SELECT_MARK_SPACING*_zoomLevel);
      int b = SELECT_MARK_SIZE;
      g.setPaint(_selColor);
      g.fillRect(abs.x-s-b,abs.y-s-b,b,b);
      g.fillRect(abs.x+s+width,abs.y-s-b,b,b);
      g.fillRect(abs.x-s-b,abs.y+s+height,b,b);
      g.fillRect(abs.x+s+width,abs.y+s+height,b,b);
    }
  }

  /** Display the name of this Graph. This method will not check if the name
   *  is visible! This task is left to <code>paint()</code>
   *
   *  @param    g     Graphics context on which to draw
   *  @param    abs   Absolute location of upper-left corner of this Graph */
  protected void paintGraph(Graphics2D g, Point abs) {
    String text = new String(_name);
    Font font = Main.SETTINGS.getGraphTitleFont();
    FontMetrics fontMetr = g.getFontMetrics(font);
    g.setFont(font);
    int rs = (int)(0.25*_roundSize);
    int textWidth = fontMetr.stringWidth(text);
    if (textWidth >= width) {                            // check if name fits
      return;                                            //   nope
    }

    g.drawString(text,(width/2)-(textWidth/2)+abs.x,  // center both
                (height/2) + abs.y                    // horz. and vert,
                + (int)(0.5*fontMetr.getMaxAscent()));
  }

  /** Redraw connections. Do not overload this method. If you like to change
   *  the appeareance of a Connection overload the Connection classs instead. */
  public void paintConnections ()
  {
    if (getOwner() != null) if (!getOwner().isGraphicalRoot()) {
      Graphics2D g = manager.getGraphics();
      for (int i=0; i<inConn.size(); i++) {
        ((Connection)inConn.get(i)).paint(g);
      }
    }
  }

  /** Draws a box containing the properties of this Graph object. The box
   *  will be centered within the rectangle specified by bounds. The method
   *  attempts to draw as much properties as the space allows. */
  public void paintProperties(Graphics2D g, String title, Rectangle bounds) {
    Config p = getProperties();
    int heightLeft = bounds.height;
    int x=0,y=0,textWidth=0;

    // get fonts and font info
    Font tFont = Main.SETTINGS.getGraphTitleFont();
    FontMetrics titleM = g.getFontMetrics(tFont);
    Font pFont = Main.SETTINGS.getGraphPropFont();
    FontMetrics propM = g.getFontMetrics(pFont);

    Point abs = bounds.getLocation();
      bounds.setLocation(0,0);

    if (p.size() > 0) {

    if ( bounds.width < titleM.stringWidth(title) ||
         bounds.height < titleM.getHeight() ) {
        System.out.println("bounds: "+bounds+", stringBounds: "+titleM.getStringBounds(title,g));
        return; // title doesn't fit
      }

      heightLeft -= titleM.getHeight()
                    - (2*LINE_SPACING)
                    - propM.getHeight();

      g.setFont(tFont);
      textWidth = titleM.stringWidth(title);
      x = (width/2)-(textWidth/2)+abs.x;
    } else {
      heightLeft = -1; // no properties to display
    }
    if (heightLeft >= 0) {
      // we can draw at least one property
      y = abs.y + titleM.getAscent();
      g.drawString(title,x,y);
      y += titleM.getDescent() + titleM.getLeading() + LINE_SPACING;
      g.drawLine(abs.x + 3,y,abs.x + bounds.width - 3, y);
      y += LINE_SPACING + propM.getAscent();
      x = abs.x + 3;
      g.setFont(pFont);
      Enumeration enum = p.list();
      int keyWidth, valueWidth;
      for (int i=0; i<p.size(); i++) {
        String key = (String)enum.nextElement();
        String value = p.getProperty(key);
        keyWidth = propM.stringWidth(key);
        valueWidth = propM.stringWidth(value);
        if (keyWidth + PROP_HORZ_SPACING + valueWidth < bounds.width) {
          g.drawString(key,x,y);
          g.drawString(value,x+PROP_INDENT,y);
        }
        y += propM.getHeight();
        heightLeft -= propM.getHeight();
        if (heightLeft < 0) break;
      }
    } else {
      // we cannot draw any properties, only draw the title
      y = (bounds.height/2) + abs.y + (titleM.getAscent()/2);
      g.drawString(title,x,y);
      g.draw(new Rectangle(x-5,y-14,textWidth+10,20));
    }
  }

  public void setBaseRoundSize(int roundSize) { _relRoundSize = roundSize; }
  public void setRoundSize(int roundSize) { _roundSize = roundSize; }
  public int getRoundSize() { return _roundSize; }
  public void setLineColor (Color color) { _lineColor = color; };
  public void setBackground (Paint paint) { _bkColor = paint; };
  public Paint getLineColor () { return _lineColor; }
  public Paint getBackground () { return _bkColor; }
  public void setName (String name) { _name = name; }
  public String getName () { return _name; }
  public String toString () {
    return _name;
  }
  public void enableClientDisplay (boolean enable) { _nameVisible = enable; }
  public boolean hasVisibleClient() { return _nameVisible; }

  /** Notify the root that the tree structure has been changed. The root will
   *  relayout the entire tree. It will then repaint the Graph. */
  public void treeStructureChanged() {
    if (getOwner() != null) {
      getOwner().treeStructureChanged();
    } else {
      // we are root
      try {
        ((GraphComposite)this).layoutAll();
      } catch (ClassCastException e) { e.printStackTrace(); }
    }
  }

  /** Notifies the root that this object changed its dimensions. */
  public void sizeChanged() {
    if (this.getOwner() == null) {
      try {
        ((GraphComposite)this).layoutAll();
      } catch (ClassCastException e) {}
    } else {
      this.getOwner().sizeChanged();
    }
  }

  /** Get the height of this Graph in pixels. */
  public int getHeight () { return height; }

  /** Get the width of this Graph in pixels. */
  public int getWidth () { return width; }

  /** Get the width of this Graph at a 100% zoom level */
  public int getBaseHeight () { return _relHeight; }
  /** Get the height of this Graph at a 100% zoom level */
  public int getBaseWidth () { return _relWidth; }

  /** Get the dimensions of this Graph at the current zoom level in pixels */
  public Dimension getSize() { return new Dimension(width,height); }

  /** Get the dimensions of this Graph at a 100% zoom level in pixels */
  public Dimension getBaseSize() { return new Dimension(_relWidth,_relHeight); }

  /** Get the X coordinate of this Graph relative to it's owners upper left
   * corner */
  public int getX () { return loc.x; }

  public int getBaseX () { return _baseLoc.x; }

  /** Get the Y coordinate of this Graph relative to it's owners upper left
   * corner
   * @deprecated Use getBaseY() */
  public int getY () { return loc.y; }

  public int getBaseY () { return _baseLoc.y; }

  /** Get the location of this Graph at the current zoom level relative to
   *  its owners upper left corner.
   *  @deprecated Use getBasePoint() */
  public Point getPoint() { return loc; }

  /** Get the location of this Graph at the current zoom level relative to
   *  its owners uppes left corner. */
  public Point getBasePoint() { return _baseLoc; }

  /** Get the (absolute) location of this Graph at the current zoom level */
  public Point getAbsoluteLocation () {
    // recursivly find the absolute location of this Graph
    if (_graphicalRoot) {
        return loc;
    } else {
      if (_owner != null) {
        Point ownerLoc = _owner.getAbsoluteLocation();
        Point thisLoc = new Point(loc);
        thisLoc.translate (ownerLoc.x,ownerLoc.y);
        return thisLoc;
      } else {
        // no owner, this Graph coordinates are absolute
        return loc;
      }
    }
  }

  /** Transform given relative coordinate to an absolute coordinate */
  public Point resolveLocation (Point rel) {
    if (rel == null) return null;
    Point abs = getAbsoluteLocation();
    abs.translate(rel.x,rel.y);
    return abs;
  }

  /** Set the Graphs relative X coordinate */
  public void setX (int x) { loc.x = x; }

  public void setBaseX(int x) { _baseLoc.x = x; }

  /** Set the Graphs relative Y coordinate */
  public void setY (int y) { loc.y = y; }

  public void setBaseY (int y) { _baseLoc.y = y; }

  /** Set the base location of this Graph. This is the location of the Graph
   *  at a 100% zoom level. This method is called by the auto-layout managers */
  public void setBaseLocation(Point loc) { _baseLoc = loc; }

  /** Set the heigth of this Graph */
  public void setHeight (int h) {
    if (h >= 0) height = h;
    sizeChanged();
  }
  /** Set the width of this Graph */
  public void setWidth (int w) {
    if (w >= 0) width = w;
    sizeChanged();
  }

  /** Change the size of this Graph */
  public void setSize(Dimension d) {
    width = (int)d.getWidth();
    height = (int)d.getHeight();
    //sizeChanged();
  }

  /** Change the size of this Graph */
  public void setBaseSize(Dimension d) {
    _relWidth = (int)d.getWidth();
    _relHeight = (int)d.getHeight();
  }

  /** Does the Graph have its dimensions set? */
  public boolean isEmpty () {
    if (_relWidth == 0 || _relHeight == 0) return true;
    else return false;
  }

  /** (re)set the location/dimension of this Graph */
  public void setFrame (int x, int y, int w, int h) {
    loc.x = x;
    loc.y = y;
    if (w >= 0) width = w;
    if (h >= 0) height = h;
  }

  public void setRect (double x, double y, double w, double h) {
    setFrame ((int)x,(int)y,(int)w,(int)h);
  }

  /**
   * Set this Graph parent. A parent is the Graph that contains this Graph
   * in case of a composite. This method will copy the GraphManager of its
   * parent by calling getGraphManager().
   *
   * @see GraphComposite
   */
  public void setOwner (GraphComposite owner) {
    _owner = owner;
  }

  /**
   * Select/deselect graph
   */
  public void setSelected (boolean selected) {
    _selected = selected;
    this.connectionShowMode(selected);
  }

  /** Is this Graph selected? */
  public boolean isSelected () { return _selected; }

  /** Set visibility */
  public void setVisible (boolean visible) {
    if (visible) {
      _visible = 0;
    } else {
      _visible = 1;
    }
  }

  /** Is this Graph visible? */
  public boolean isVisible () {
    //return _visible && canBeDrawn();
    return (_visible==0);
  }

  /** Returns the Graphs parent. Null if it hasn't any */
  public GraphComposite getOwner () { return _owner; }

  // --- -- - - - - - -- - - -- - -- -   --  - - - -- -   -- - - -- - - --- -
  // Methods used for connecting this Graph to other Graphs
  // - -- -- - - -- - - -- - - - - - -- -- -- -- --- ----- -- -- - -- - --- -

  /** Provides access to the incoming Connections */
  public Connection getInConnection (int index) {
    if (index >= inConn.size()) return null;
    return (Connection)inConn.get(index);
  }

  /** Provides access to the outgoing Connections */
  public Connection getOutConnection (int index) {
    if (index >= outConn.size()) return null;
    return (Connection)outConn.get(index);
  }

  /** Retreive the number of incoming connections */
  public int nrInConnections ()
  {
    return inConn.size();
  }

  /** Retreive the number of outgoing connections */
  public int nrOutConnections ()
  {
    return outConn.size();
  }

  /** Notify this Graph that it is part of a Connection between two Graphs */
  public void registerConnection (Connection connection)
  {
    if (connection.graphIn == this) inConn.add(connection);
    else if (connection.graphOut == this) outConn.add(connection);
    //TODO: else throw an exception
  }

  /** Connect this graph to the specified graph using the specified connection.*/
  public Connection connectTo (Graph graph, Connection connection) {
    connection.graphOut = this;
    connection.graphIn = graph;
    graph.registerConnection(connection);
    outConn.add(connection);
    return connection;
  }

  /** Create a unidirectional connection. A handler to the created Connection
   * is returned. */
  public Connection connectTo (Graph graph)
  {
    Connection connection = new Connection (this,graph);
    return connectTo(graph,connection);
  }

  /** Determine if this Graph is connected to the selected Graph in any
   * direction */
  public boolean isConnectedTo (Graph graph)
  {
    for (int i=0; i<outConn.size(); i++)
    {
      if (((Connection)outConn.get(i)).graphIn == graph) return true;
    }
    for (int i=0; i<inConn.size(); i++)
    {
      if (((Connection)inConn.get(i)).graphOut == graph) return true;
    }
    return false;
  }

  /** Determine if this Graph is already connected to any other Graph (input) */
  public boolean hasInputConnections () { return inConn.size() > 0; }

  /** Determine if this Graph is already connected to any other Graph (output) */
  public boolean hasOutputConnections () { return outConn.size() > 0; }

  /**
   * Ask this Graph for a connection coordinate in specified orientation.
   * The returned coordinate is absolute. If a point cannot be determined it
   * returns null.
   */
  public Point getConnectionPoint (boolean incoming, Connection con) {
    Point point;
    // are we visible?
    if ((_visible==0) && canBeDrawn()) {
      if (incoming) {
        point = resolveLocation (new Point(0,height/2));
      } else {
        point = resolveLocation (new Point(width,height/2));
      }
    } else {
      if (_owner == null) return null;        // can't create connection point
      point = _owner.getConnectionPoint(incoming,con);
    }
    return point;
  }

  /** Returns 'this' when this Graph contains the specified point. If not
   * it returns null. */
  public Graph containerOf (Point p) {
    Point absCoor = getAbsoluteLocation();
    if (p.getX() >= absCoor.getX() && p.getX() <= absCoor.getX()+width &&
      p.getY() >= absCoor.getY() && p.getY() <= absCoor.getY()+height) {
      return this;
    } else return null;
  }

  /** Add this to bounded when the Graph lies within the specified bounds */
  public void enumBoundedGraphs (Rectangle bounds, Vector bounded) {
    Point absC = getAbsoluteLocation();
    Rectangle absR = new Rectangle(absC,new Dimension(width,height));
    if (bounds.contains(absR)) {
      bounded.add(this);
    }
  }

  /** Sets the composite tree level of this Graph.  */
  public void setTreeLevel (int level) {
    _treeLevel = level;
    manager.suggestTreeHeight(_treeLevel);
  }

  /** Returns the level of this Graph in the composite tree. */
  public int getTreeLevel () { return _treeLevel; }

  /** Determines if this Graph can be drawn. It checks if the Graph has enough
   *  space (taken the given zoomlevel) to draw itself. Override this function
   *  to specify what conditions should be met for the Graph to draw itself. */
  public boolean canBeDrawn() { return true; }

  public boolean connectionShowMode(boolean bold) {
    Connection c;
    for (int i=0; i<inConn.size(); i++) {
      c = (Connection)inConn.get(i);
      c.setSelected(bold);
    }
    for (int i=0; i<outConn.size(); i++) {
      c = (Connection)outConn.get(i);
      c.setSelected(bold);
    }
    return true;
  }

  /** Reset the dimensions of this Graph */
  protected void resetDim () { height=0; width=0; _relHeight=0; _relWidth=0; }

  /** Returns all interesting properties of this Graph. Overload this method
   *  to add your own interesting properties. */
  public Config getProperties() {
    Config p = new Config();
    p.putString("Name",_name);
    return p;
  }

  /** Overload this function if you want to relate an icon with this Graph */
  public Icon getIcon() {
    return null;
  }

  final static int LINE_SPACING = 5;
  final static int PROP_HORZ_SPACING = 50;
  final static int PROP_INDENT = 100;
}