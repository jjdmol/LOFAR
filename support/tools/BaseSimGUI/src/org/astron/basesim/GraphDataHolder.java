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
import kiwi.util.Config;

/**
 * A DataHolder is a specific Graph that can be added to the Step and Simul
 * composites. A DataHolder can be either used for input or output. Specific
 * DataHolders should inherit from this class.
 */
public class GraphDataHolder extends Graph {

  /**
   * Is this an input DataHolder?
   */
  boolean _inputDataHolder;

  public GraphDataHolder() {
    super(null,0,0,20,33,"DataHolder"+followUp++);
    defaultSettings();
  }

  public GraphDataHolder(GraphManager man, boolean input) {
    super(man,0,0,20,33,"DataHolder"+followUp++);
    _inputDataHolder = input;
    defaultSettings();
  }

  public GraphDataHolder(GraphManager man, String name,boolean input) {
    super(man,0,0,20,33,name);
    _inputDataHolder = input;
    defaultSettings();
  }

  public GraphStep getOwnerStep() {
    return (GraphStep)getOwner().getOwner();
  }

  void defaultSettings()
  {
    setBackground(Color.orange);
    setBaseRoundSize(0);
  }

  /**
   * Create DataHolder connections. Only one connection can be made per
   * DataHolder. When an attemption is made to create multiple connections
   * a warning Message will be send to all MessageListeners
   */
  public Connection connectTo (Graph graph, Connection connection)
  {
    connection.graphOut = this;
    connection.graphIn = graph;
    if (! (graph instanceof GraphDataHolder)) {
      manager.fireMessage(new MessageGraph(this,
        "Attempt to connect "+this+" to "
        + graph +" failed (they are incompatible)",MessageGraph.WARNING));
      return null;
    }
    if (!this.hasOutputConnections()) {
      if (!graph.hasInputConnections()) {
        graph.registerConnection(connection);
        outConn.add(connection);
        return connection;
      } else {
        manager.fireMessage(new MessageGraph(this,
          "Attempt to connect "+this+" to "+graph+" failed (" + graph
            + " already has its input connected)",MessageGraph.WARNING));
        return null;
      }
    } else {
      manager.fireMessage(new MessageGraph(this,
        "Attempt to connect "+this+" to "+graph+" failed (" + this
          + " already has an output connection)",MessageGraph.WARNING));
      return null;
    }
  }

  /**
   * Create DataHolder connections. Only one connection can be made per
   * DataHolder. When an attemption is made to create multiple connections
   * a warning Message will be send to all MessageListeners
   */
  public Connection connectTo (Graph graph)
  {
    Connection connection = new Connection (this,graph);
    return connectTo(graph,connection);
  }

  public void setInput(boolean input) { _inputDataHolder = input; }

  /** Is this an input DataHolder? */
  public boolean isInput () { return _inputDataHolder; }

  /** Is this an output DataHolder? */
  public boolean isOutput () { return !_inputDataHolder; }

  public boolean canBeDrawn() {
    //if (getZoomLevel() < 0.4) return false; else return true;
    //if (getOwner().getVerticalSpacing() < 10) return false; else return true;
    return true;
  }

  /** Alter size of this GraphDataHolder */
  public void setTreeLevel (int level) {
    super.setTreeLevel(level);
    setBaseSize(new Dimension ((int)(20*(1.0/level*6)),(int)(33*(1.0/level*6))));
  }

  public String getClassName() { return ""; }

  public Config getProperties() {
    Config p = super.getProperties();
    p.putString("Class",getClassName());
    return p;
  }
}