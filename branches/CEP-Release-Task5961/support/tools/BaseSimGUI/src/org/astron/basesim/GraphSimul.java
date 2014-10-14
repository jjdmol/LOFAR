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

import java.util.*;
import java.awt.*;
import org.astron.util.*;
import kiwi.util.Config;

public class GraphSimul extends GraphStep {

  final static Color defBackground = new Color(180,190,240);

  LayoutSimul _layoutSimul = new LayoutSimul();

  /** The WorkHolder of a Simul is not part of the diagram, but it is visible
   *  when the Graph tree is displayed in a JTree. So we store it here.
   *  @see BaseSimTreeModel*/
  private GraphWorkHolder _gwh;

  public GraphSimul() {
    super (null,"GraphSimul"+followUp++);
    defaultSettings();
  }

  public GraphSimul(GraphManager man) {
    super (man,"GraphSimul"+followUp++);
    defaultSettings();
  }

  public GraphSimul(GraphManager man, String name) {
    super (man,name);
    defaultSettings();
  }

  private void defaultSettings() {
    setBackground(defBackground);
    ((GraphComposite)this.getGraph(1)).setLayoutController(_layoutSimul);
    getInnerComposite().setDimCtrl(STATE_INDEPENDENT,new DimCtrlStepCenter());
    getDimCtrl().setSpacing(10,100);
  }

  /** Add WorkHolder to this Simul object. Although not visible in the
   *  GraphSimul, the WorkHolder is stored in a member variable. (We must
   *  store it, because the JTree does show it.) */
  public void setWorkHolder(GraphWorkHolder workHolder) {
    _gwh = workHolder;
    // ugly workaround. needed for BaseSimTreeModel
    GraphComposite gc = new GraphComposite(getGraphManager());
    gc.setOwner(this);
    _gwh.setOwner(gc);
    _workHolderIsSet = true;
  }

  /** Returns the GraphWorkHolder of this GraphSimul. Returns null if not set.*/
  public GraphWorkHolder getWorkHolder() { return _gwh; }

  /** Returns true if this GraphSimul has a WorkHolder */
  public boolean hasWorkHolder() { return _gwh != null; }

  public int numberOfSteps() { return _compCore.numberOfGraphs(); }

  public void addStep (GraphStep step) {
    getInnerComposite().addGraph(step);
  }

  public GraphStep getStep(int index) {
    return (GraphStep)getInnerComposite().getGraph(index);
  }

  public void setZoomLevel (double level) {
    super.setZoomLevel(level);
    if (_gwh != null) getWorkHolder().setZoomLevel(level);
  }

  /** Returns all interesting properties of this GraphStep. Overload this method
   *  to add your own interesting properties. */
  public Config getProperties() {
    Config p = super.getProperties();
    p.putInt("numberOfSteps",numberOfSteps());
    return p;
  }
}