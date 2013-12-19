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
import java.util.*;

/**
 * A Step is a kind of composite that holds a (centered) Workholder,
 * a set of input DataHolders and a set of output DataHolders.
 * <blockquote>
 * <em>Note: Although graphically a GraphStep CONTAINS DataHolders, in the
 * LofarSim model it's the WorkHolder that contains the DataHolders. This
 * gives a little trouble when parsing the model file when a WorkHolder
 * is encountered which should be placed in a Step which might not yet
 * be available. This can be overcome by parsing from the highest level (Simuls)
 * down to the lower levels (Steps->WorkHolders->DataHolders)</em>
 * </blockquote>
 */
public class GraphStep extends GraphComposite {

  /**
   * Is a Workholder set and layed out in this Step?
   */
  boolean _workHolderIsSet;
  /**
   * Is a Paramholder set and layed out in this Step?
   */
  boolean _paramHolderIsSet;

  protected GraphComposite _compInput; // contains input DataHolders
  protected GraphComposite _compOutput; // contains output DataHolders
  protected GraphComposite _compCore; // contains WorkHolder or Steps

  private String _node;

  boolean _init;

  public GraphStep() {
    super (null,0,0,"GraphStep"+followUp++);
    defaultSettings();
  }

  public GraphStep(GraphManager man) {
    super (man,0,0,"GraphStep"+followUp++);
    defaultSettings();
  }

  public GraphStep(GraphManager man, String name) {
    super (man,0,0,name);
    defaultSettings();
  }

  private void defaultSettings ()
  {
    setBackground(Color.lightGray);
    setLineColor(Color.black);
    setBaseRoundSize(20);
    _paramHolderIsSet = false;
    _init = true; // for Step specific initialization
    _compInput = new GraphComposite(getGraphManager());
    _compInput.getDimCtrl().setSpacing (30,20);
    _compInput.getDimCtrl().setSpacingFactor(0.05f,false);
    _compOutput = new GraphComposite(getGraphManager());
    _compOutput.getDimCtrl().setSpacing (30,20);
    _compOutput.getDimCtrl().setSpacingFactor(0.05f,false);
    _compInput.setLayoutController(
      new LayoutDataHolder(LayoutDataHolder.STEP_INPUT));
    _compInput.enableBorder(false);
    _compInput.enableClientDisplay(false);
    _compOutput.setLayoutController(
      new LayoutDataHolder(LayoutDataHolder.STEP_OUTPUT));
    _compOutput.enableBorder(false);
    _compOutput.enableClientDisplay(true);
    _init = false;

    _compCore = new GraphCompositeInner(getGraphManager());
    _compCore.setLayoutController(new LayoutCentered());
    _compCore.enableBorder(false);
    _compCore.enableClientDisplay(true);

  // -Note-------------------------------------------------------------------
  // For GraphSimul classes the vertical and horizontal spacings between
  // its children are determined by its LayoutController. So the horizontal
  // and vertical spacings set here for _compCore will not be applied to
  // GraphSimul objects.
  // ------------------------------------------------------------------------

    _compCore.getDimCtrl().setSpacing(80,16);
    _compCore.getDimCtrl().setSpacingFactor(0.10f,false);

    addGraph(_compInput);
    addGraph(_compCore);
    addGraph(_compOutput);
    setLayoutController(new LayoutFlow(LayoutFlow.ALIGN_HORZ));
    getDimCtrl().setSpacing(0,0);
    setBaseSize(new Dimension(300,0));
  }

  /**
   * Determine if this Step has a WorkHolder
   */
  public boolean hasWorkHolder ()
  {
    return _workHolderIsSet;
  }

  /**
   * Add a WorkHolder. Only one WorkHolder can exist in a Step.
   */
  public void setWorkHolder (GraphWorkHolder workHolder)
  {
    if (_workHolderIsSet) return;
    _compCore.addGraph (workHolder);
    _workHolderIsSet = true;
  }

  /** Get the WorkHolder of this GraphStep. Returns null if not set.*/
  public GraphWorkHolder getWorkHolder() {
    return (_compCore.numberOfGraphs()>0) ?
           (GraphWorkHolder)_compCore.getGraph(0) : null;
  }

  public GraphSimul getOwnerSimul() {
    return (GraphSimul)getOwner().getOwner();
  }

  /**
   * Add a DataHolder.
   */
  public void addDataHolder (GraphDataHolder dataHolder)
  {
    if (dataHolder.isInput()) _compInput.addGraph(dataHolder);
    else _compOutput.addGraph(dataHolder);
  }

  /**
   * Add a ParamHolder. Only one ParamHolder can exist in a Step
   */
  public void addParamHolder (GraphDataHolder paramHolder)
  {
    addGraph (paramHolder);
    _paramHolderIsSet = true;
  }

  /**
   * Connect this Step to another Step. There are two possibilities,
   * one is a connection between two Steps on the same level, the other is a
   * connection between a parent and it's child.
   */
  public void connectTo (GraphStep step)
  // REVISIT: return value should be 'Connection'?
  {
    boolean connectionsCreated = false;

    GraphDataHolder gdhThis[], gdhThat[];
    if (this == step.getOwner().getOwner()) {
      // situation where a simul's input is connected to the input of one
      // of its children (in->in)
      gdhThis = getInputDataHolders();
      gdhThat = step.getInputDataHolders();
    } else if (getOwner().getOwner() == step) {
      // situation where a simul's ouput is connected to its parent output
      // (out->out)
      gdhThis = getOutputDataHolders();
      gdhThat = step.getOutputDataHolders();
    } else {
      // connection between steps on the same level (out->in)
      gdhThis = getOutputDataHolders();
      gdhThat = step.getInputDataHolders();
      (super.connectTo(step)).setVisible(false);
    }
    int thatIx=0;
    if (gdhThat.length == 0) return;
    for (int i=0; i<gdhThis.length; i++)
    {
      if (!gdhThis[i].hasOutputConnections()) {
        while (gdhThat[thatIx].hasInputConnections())
        {
          if (++thatIx>=gdhThat.length) {
            if (!connectionsCreated) manager.fireMessage(
              new MessageGraph(this,"Could not connect "+this+" to "+step+"."
                               + " No input dataholders available."));
            return ;
          }
        }
        gdhThis[i].connectTo(gdhThat[thatIx++]);
        connectionsCreated = true;
        if (thatIx >= gdhThat.length) break;
      }
    }
    if (!connectionsCreated) manager.fireMessage(
      new MessageGraph(this,"Could not connect "+this+" to "+step+"."
                       + " No output dataholders available."));
  }

  /** Returns input GraphDataHolder count */
  public int nrInputDataHolders() { return _compInput.numberOfGraphs(); }

  /** Returns output GraphDataHolder count */
  public int nrOutputDataHolders() { return _compOutput.numberOfGraphs(); }

  /** Get a list of input DataHolders. Used for connection between Steps */
  public GraphDataHolder[] getInputDataHolders () {
    GraphDataHolder gdh[] = new GraphDataHolder[_compInput.numberOfGraphs()];
    for (int i=0;i<gdh.length;i++) gdh[i] = (GraphDataHolder)_compInput.getGraph(i);
    return gdh;
  }

  /** Get a list of output DataHolders. Used for connection between Steps */
  public GraphDataHolder[] getOutputDataHolders () {
    GraphDataHolder gdh[] = new GraphDataHolder[_compOutput.numberOfGraphs()];
    for (int i=0;i<gdh.length;i++) gdh[i] = (GraphDataHolder)_compOutput.getGraph(i);
    return gdh;
  }

  public GraphComposite getInputDataHolderComposite() {
    return _compInput;
  }

  public GraphComposite getOutputDataHolderComposite() {
    return _compOutput;
  }

  public GraphComposite getInnerComposite() {
    return _compCore;
  }

  /** Overloaded from GraphComposite. Passes the state change request to its
   *  inner composite. */
  public void setState(int state) {
    _compCore.setState(state);
  }

  /** Overloaded from GraphComposite. Passes the current state request to its
   *  inner composite. */
  public int getState() {
    return _compCore.getState();
  }

  /**
   * Get a connection point to this Step. The point is absolute. If the point
   * cannot be determined it will return null.
   */
  public Point getConnectionPoint (boolean incoming, Connection con)
  {
    // are we visible?
    if (isVisible() && canBeDrawn()) {
      if (incoming) {
        Graph graph = con.graphIn;
        if (((GraphDataHolder)graph).isOutput()) return null;
        Point pC = graph.getAbsoluteLocation();
        Point pT = this.getAbsoluteLocation();
        pT.y = (int)(pC.getY()+graph.getHeight()/2);
        return pT;
      } else {
        Graph graph = con.graphOut;
        if (((GraphDataHolder)graph).isInput()) return null;
        Point pC = graph.getAbsoluteLocation();
        Point pT = this.getAbsoluteLocation();
        pT.x += getWidth();
        pT.y = (int)(pC.getY()+graph.getHeight()/2);
        return pT;
      }
    } else {
      return null;
    }
  }

  public boolean canBeDrawn() {
    return true;
    //if (getOwner().getVerticalSpacing() <= 1) return false; else return true;
  }

  /** Enable/disable showing step info. A GraphStep is build of tree
   *  GraphComposite objects. Calling this method will set the state of the
   *  middle GraphComposite to either STATE_INDEPENDENT or STATE_DRAWCHILDREN */
  public void showStepInfo(boolean enable) {
    if (enable) {
      if (_compCore.getState() == STATE_INDEPENDENT) return; // already in state
      _compCore.setState(STATE_INDEPENDENT);
    } else {
      if (_compCore.getState() == STATE_DRAWCHILDREN) return; // already in state
      _compCore.setState(STATE_DRAWCHILDREN);
    }
  }

  public void setNode(String node) { _node = node; }
  public String getNode() { return _node; }

  protected void paintGraph(Graphics2D g, Point abs) {
    String origName = new String(getName());
    if (_node != null) setName(getName() + " (" + _node + ")");
    super.paintGraph(g,abs);
    setName(origName);
  }

  /** This class represent the inner composite that will, in case of a GraphStep,
   *  display the WorkHolder, or, in case of a Graphsimul display its contained
   *  Steps or its WorkHolder. */
  private class GraphCompositeInner extends GraphComposite {
      public GraphCompositeInner(GraphManager man) {
        super(man);
      }

      public GraphCompositeInner(GraphManager man, String name) {
        super(man,name);
      }

      public GraphCompositeInner(GraphManager man, int x, int y, String name) {
        super(man,x,y,name);
      }
      /** Lets the WorkHolder of this simul/step do the drawing.
       *  A Simul will display a picture together with some info when
       *  its Steps are invisible. */
      protected void paintGraph(Graphics2D g, Point abs) {
        GraphWorkHolder wh = getWorkHolder();
        if (wh != null) wh.paintGraph(g,abs);
      }
  }
}
