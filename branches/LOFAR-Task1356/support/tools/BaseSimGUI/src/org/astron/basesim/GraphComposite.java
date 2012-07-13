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
 * Title
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      Astron
 * @author Robbert Dam
 * @version 1.0
 */

import java.util.*;
import java.awt.*;
import kiwi.util.Config;

/**
 * This class represents a composition of multiple Graphs. It contains
 * methods for adding Graph to the composite (these functions will resize
 * the composite Graph according to the dimensions of the added Graph)
 *
 * A composition has its own coordinate system with (0,0) set to the
 * upper left coordinates of the composition.
 *
 * The default GraphComposite displays a filled rectangle to visualize its
 * boundaries. You can disable enable this property with enableBorder().
 *
 * A GraphComposite can be in two different states. In the first state
 * it will not paint its children and it will calculate its own coordinates.
 * In this state the GraphComposite can use its entire client area to draw on.
 * In the second state it will draw its children. The methode setState()
 * can switch between these states.
 *
 * The layout of components within the GraphComposite is controlled by a
 * LayoutController. The dimensions of the GraphComposite are controlled
 * by a DimensionController. The default DimensionController inflates the
 * GraphComposite so that its children just fit. Every state has a
 * DimensionController attached to it.
 */
public class GraphComposite extends Graph {

  final static int NR_OF_STATES = 2;

  /** State in which the GraphComposite will not draw its children */
  final static int STATE_INDEPENDENT = 0;
  /** State in which the GraphComposite will draw its children */
  final static int STATE_DRAWCHILDREN = 1;

  /** Holds the GraphComposites' children */
  protected ArrayList _graphList;
  /** The manager that handles the layout of components in this Graph */
  protected LayoutController _layoutController;
  /** The managers that handle the dimensions of this GraphComposite */
  protected DimensionController dimController[] =
    new DimensionController[NR_OF_STATES];
  /** Current horizontal spacing between Graphs */
  private int _horzSpacing;
  /** Current vertical spacing between Graphs */
  private int _vertSpacing;
  /** Used by function getFirstChild() and getNextChild() */
  private int _currentChild=0;
  /** Current state */
  private int state = STATE_DRAWCHILDREN;

  public GraphComposite(GraphManager man) {
    super(man,0,0,"GraphComposite"+followUp++);
    defaultSettings();
  }

  public GraphComposite(GraphManager man, String name) {
    super (man,0,0,name);
    defaultSettings();
  }

  /** Create an empty composite with no dimensions. */
  public GraphComposite(GraphManager man, int x, int y, String name) {
    super (man,x,y,name);
    defaultSettings();
  }

  private void defaultSettings () {
    _graphList = new ArrayList ();
    _horzSpacing = 0;
    _vertSpacing = 0;
    setDimCtrl(STATE_DRAWCHILDREN,new DimensionController()); // default 'inflater'
  }

  /** Select layout */
  public void setLayoutController (LayoutController layoutController) {
    _layoutController = layoutController;
    _layoutController.setGraphComposite(this);
  }

  /** Retreive layout manager. Null if not set. */
  public LayoutController getLayoutController () { return _layoutController; }

  /** Registers DimensionController with this component. The state specifies
   *  the GraphComposite state in which this DimensionController will be
   *  active. This can be STATE_DRAWCHILDREN or STATE_INDEPENDENT. */
  public void setDimCtrl (int state, DimensionController dimController) {
    this.dimController[state] = dimController;
    this.dimController[state].setGraphComposite(this);
  }

  /** Returns registered DimensionController for a specified state. The
   *  state can be STATE_DRAWCHILDREN or STATE_INDEPENDENT. It returns null
   *  if no DimensionController has been set for the specified state. */
  public DimensionController getDimCtrl(int state) { return dimController[state]; }

  /** Returns the DimensionController for the current state. */
  public DimensionController getDimCtrl() { return dimController[state]; }

  /** Set visibility. The method sets the same value to all its children when
   *  its state flag is set to STATE_DRAWCHILDREN. */
  public void setVisible (boolean visible) {
    super.setVisible(visible);
    if (state == STATE_DRAWCHILDREN) {
      for (int i=0; i<numberOfGraphs(); i++) {
        getGraph(i).setVisible(visible);
      }
    }
  }

  protected void transformDimensions() {
    super.transformDimensions();
    _horzSpacing = (int)(getZoomLevel() * getDimCtrl(state).getHorizontalSpacing());
    _vertSpacing = (int)(getZoomLevel() * getDimCtrl(state).getVerticalSpacing());
  }

  /** Recalc the dimensions of this Graph using the given zoom level. */
  public void setZoomLevel (double level) {
    super.setZoomLevel(level);
    for (int i=0; i<_graphList.size(); i++) {
      ((Graph)_graphList.get(i)).setZoomLevel(level);
    }
  }

  /** Change state of this GraphComposite. Can be STATE_INDEPENDANT,
   *  STATE_DRAWCHILDREN. The method will inform the GraphManager that it
   *  should relayout the entire tree, because of a size change. */
  public void setState(int state, boolean recurse) {
    setState(state);
    if (recurse) {
      for (int i=0; i<numberOfGraphs(); i++) {
        Graph graph = getGraph(i);
        if (graph instanceof GraphComposite) {
          ((GraphComposite)graph).setState(state,true);
        }
      }
    }
  }

  /** Change state of this GraphComposte. Can be STATE_INDEPENDANT,
   *  STATE_DRAWCHILDREN. The method will inform the GraphManager that it
   *  should relayout the entire tree, because of a size change. */
  public void setState(int state) {
    if (dimController[state] == null) {
      System.out.println("Warning: No dimension controller exists for specified state.");
      return;
    }
    this.state = state;
    getGraphManager().setDimensionsChanged();

    if (this.state == STATE_INDEPENDENT) {
      for (int i=0; i<numberOfGraphs(); i++) {
        getGraph(i).setVisible(false);
      }
    } else
    if (this.state == STATE_DRAWCHILDREN) {
      for (int i=0; i<numberOfGraphs(); i++) {
        getGraph(i).setVisible(true);
      }
    }
  }

  public void setGraphManager(GraphManager man) {
    super.setGraphManager(man);
    for (int i=0; i<numberOfGraphs(); i++) {
      getGraph(i).setGraphManager(man);
    }
  }

  /** Returns either STATE_INDEPENDANT or STATE_DRAWCHILDREN. */
  public int getState() { return state; }

  /** Paint this composite. When its is in state STATE_DRAWCHILDREN it will
   *  calls <code>paint()</code> on all its children,
   *  after some checks have been made (visibility, withinh clip bounds, etc)
   *  When state is STATE_INDEPENDANT, it will call its paintGraph method. */
  public boolean paint () {
    if (manager == null) return false;
    if (manager.getGraphics() == null) return false;
    if ((this.numberOfGraphs() == 0)) return  false; // empty composite
    Graphics2D g = manager.getGraphics();
    Point point = getAbsoluteLocation();        // Resolve location
    if (hasVisibleBorders()) paintBorder(g,point);
    if (state == STATE_INDEPENDENT) {
      if (hasVisibleClient()) paintGraph(g,point);
      return true;
    } else {
      if (!isVisible()) return false;
      for (int i=0; i<_graphList.size(); i++) {
        Graph graph = (Graph)_graphList.get(i);
        if (!graph.isVisible()) continue;
        if (graph.isEmpty()) continue;
        if (!graph.canBeDrawn()) continue;
        Rectangle rect = manager.getGraphics().getClipBounds();
        Point p = graph.getAbsoluteLocation();
        //REVISIT: avoid object creation at this point
        if (!rect.intersects(new Rectangle(p,       // within bounds?
          new Dimension(graph.getWidth(),graph.getHeight())))) continue;
        graph.paint();
      }
      return true;
    }
  }

  /** Paint all connections and call <code>paintConnections()</code> on all
   *  its children. root must be set to true */
  public void paintConnections () {
    super.paintConnections();
    for (int i=0; i<numberOfGraphs(); i++) getGraph(i).paintConnections();
  }

  /** Display the name of this Graph.
   *  @param    abs   Absolute location of upper-left corner of Graph */
  protected void paintGraph(Graphics2D g, Point abs) {
    Font font = new Font("arial",Font.PLAIN,(int)(11));
    FontMetrics fontMetr = g.getFontMetrics(font);
    g.setFont(font);
    int rs = (int)(0.25*getRoundSize());
    int textWidth = fontMetr.stringWidth(getName());
    if (textWidth >= getWidth()) {                            // check if name fits
      return;                                                 //   nope
    }
    g.drawString(getName(),(getWidth()/2)-(textWidth/2)+abs.x,    // center horizontally
                 abs.y+fontMetr.getMaxAscent()+rs);
  }

  /** Relayout subtree of this GraphComposite. Should only be called on
   *  the root, or you'll mess up your layout! */
  public void layoutAll () {
    if (_layoutController == null) return;
    _layoutController.preprocess();
    layoutChildren();
    if (_layoutController.canBeDone()) _layoutController.doLayout();
    dimController[state].setDimensions();
  }

  /** Calls layoutAll() on all its children. */
  private boolean layoutChildren () {
    for (int i=0; i<_graphList.size(); i++) {
      Graph graph = (Graph)_graphList.get(i);
      if (graph instanceof GraphComposite)
        ((GraphComposite)graph).layoutAll();
    }
    return true;
  }

  /** Sets the composite tree level of this Graph.  */
  public void setTreeLevel (int level) {
    super.setTreeLevel(level);
    for (int i=0; i<numberOfGraphs(); i++) {
      getGraph(i).setTreeLevel(level+1);
    }
  }

  /** set a Graph at a specific index. Index should be a value in the range
   *  0 - numberOfGraphs()-1 */
  public void setGraph(Graph graph, int index) {
    if ( (index < 0) || (index >= numberOfGraphs()) ) return;
    if (state == STATE_INDEPENDENT) graph.setVisible(false);
    if (!isVisible()) graph.setVisible(false);
    graph.setOwner(this);
    _graphList.set(index,graph);
  }

  /** Add a Graph to the composite. */
  public void addGraph (Graph graph) {
    if (state == STATE_INDEPENDENT) graph.setVisible(false);
    if (!isVisible()) graph.setVisible(false);
    graph.setOwner (this);
    _graphList.add(graph);
  }

  /** Remove a Graph from the composite. */
  public void removeGraph (Graph graph) {
    graph.setOwner(null);
    _graphList.remove(graph);
    //treeStructureChanged();
  }

  /** Remove a Graph from the composite. */
  public void removeGraph (int index) {
    if (index < _graphList.size()) {
      this.getGraph(index).setOwner(null);
      _graphList.remove(index);
      //treeStructureChanged();
    } else {
      System.err.println("GraphComposite: Tried to remove nonexis. Graph");
    }
  }

  /** Remove all Graph from the composite */
  public void removeAllGraphs() {
    for (int i=0; i<_graphList.size(); i++) {
      this.getGraph(i).setOwner(null);
    }
    _graphList.clear();
  }

  /** Replace a Graph at a given index with the specified Graph */
  public void replaceGraph(int index, Graph graph) {
    _graphList.remove(index);
    graph.setOwner(this);
    _graphList.add(index,graph);
  }

  /** This method provides access to the Graph in the composition by index */
  public Graph getGraph (int index) {
    return (index < _graphList.size()) ? (Graph)_graphList.get(index) : null;
  }

  /** Returns the number of Graphs in this composition */
  public int numberOfGraphs () { return _graphList.size(); }

  /**
   * This method returns the index of the specified Graph in this composite.
   * If the Graph is not part of this composite it returns -1.
   */
  public int getGraphIndex (Graph graph) {
    for (int i=0; i<_graphList.size(); i++)
      if (_graphList.get(i)==graph) return i;
    return -1;
  }

  /** Get a connection point. A basic GraphComposite cannot determine a
   * connection point, so it will always pass the request to its owner.
   * When no connection point could be determined, the method returns null.
   * Overload this method when you want a composite to return a connection
   * point. */
  public Point getConnectionPoint (boolean incoming, Connection con) {
    GraphComposite owner = getOwner();
    if (owner != null) return owner.getConnectionPoint(incoming,con);
    else return null;
  }

  /** Returns the Graph which contains the specified point. Return null if
   * not any of the Graphs contains this point */
  public Graph containerOf (Point p) {
    if (super.containerOf(p)!=null) {
      Graph graph;
      for (int i=0; i<_graphList.size(); i++) {
        graph = ((Graph)_graphList.get(i)).containerOf(p);
        if (graph != null) if (graph.hasVisibleBorders()&&
            graph.canBeDrawn()) return graph; else break;
      }
      return this;
    }
    return null;
  }

  /** Add this to bounded when the Graph lies within the specified bounds.
   *  Recusivly calls enumBoundedGraphs() on all its children. */
  public void enumBoundedGraphs (Rectangle bounds, Vector bounded) {
    super.enumBoundedGraphs(bounds,bounded);
    if (isVisible()) {
      for (int i=0; i<numberOfGraphs(); i++) {
        getGraph(i).enumBoundedGraphs(bounds,bounded);
      }
    }
  }

  /** Does this GraphComposite contain the specified Graph? */
  public boolean contains(Graph graph) {
    for (int i=0; i<_graphList.size(); i++) {
      if ((Graph)_graphList.get(i) == graph) return true;
    }
    return false;
  }

  /** Return width of largest composite Graph (based at zoom level of 100%) */
  public int maxHeightGraph() {
    int maxHeight=0;
    for (int i=0; i<this.numberOfGraphs(); i++) {
      Graph graph = this.getGraph(i);
      if (graph.isVisible()) {
        if (maxHeight < graph.getBaseHeight()) {
          maxHeight = graph.getBaseHeight();
        }
      }
    }
    return maxHeight;
  }

  /** Return height of heighest composite Graph (based at zoom level of 100%) */
  public int maxWidthGraph() {
    int maxWidth=0;
    for (int i=0; i<this.numberOfGraphs(); i++) {
      Graph graph = this.getGraph(i);
      if (graph.isVisible()) {
        if (maxWidth < graph.getBaseWidth()) {
          maxWidth = graph.getBaseWidth();
        }
      }
    }
    return maxWidth;
  }
}